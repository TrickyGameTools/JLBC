// Lic:
// JCR Lua Compile & Bundle
// Compiler & Bundler
// 
// 
// 
// (c) Jeroen P. Broks, 2022
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// Please note that some references to data like pictures or audio, do not automatically
// fall under this licenses. Mostly this is noted in the respective files.
// 
// Version: 22.10.06
// EndLic

// JCR Lua Compile & Bundle

#include <iostream>
#include <memory>

#include <lua.hpp>

#include <QuickString.hpp>
#include <ArgParse.h>
#include <QuickTypes.hpp>
#include <QuickStream.hpp>
#include <FileList.hpp>

#include <jcr6_core.hpp>
using namespace std;
using namespace TrickyUnits;
using namespace jcr6;

FlagConfig Flags{};
ParsedArg Args{};
shared_ptr<JT_Create> Out{ nullptr };
string OutBundle{};
//string dest;
vector<char> OutBuf;

#pragma region Errors
uint64 Errors{ 0 };
void Error(string msg, bool fatal=false) {
	printf("\x1b[31mERROR #%05llu:\x1b[0m  %s\n", ++Errors, msg.c_str());
	if (fatal) exit(1);
}
#pragma endregion

#pragma region Compile
bool err{ false };
int LuaPaniek(lua_State* L) {
	string Trace{};
	Error("Lua Error!");
	cout << lua_gettop(L) << "\n";
	for (int i = 1; i <= lua_gettop(L); i++) {
		cout << "Arg #" << i << "\t";
		switch (lua_type(L, i)) {
		case LUA_TSTRING:
			cout << "String \"" << luaL_checkstring(L, i);
			Trace += luaL_checkstring(L, i); Trace += "\n";
			break;
		case LUA_TNUMBER:
			cout << "Number " << luaL_checknumber(L, i);
		case LUA_TFUNCTION:
			cout << "Function";
		default:
			cout << "Unknown: " << lua_type(L, i);
			break;
		}
		cout << "\n";
	}
	//Error("", false, true);
	//exit(11);
	err = true;
	return 0;
}

int DumpLua(lua_State* L, const void* p, size_t sz, void* ud) {
/*
	if (Out) {
		cout << "Bundling:  " << dest << endl;
		auto BT{ Out->StartEntry(dest) };
		auto pcp{ (char*)p };
		for (size_t i = 0; i < sz; ++i) BT->Write(pcp[i]);
		BT->Close();	
	} else {
		cout << "Writing:   " << dest << endl;
		auto BT{ WriteFile(dest) };
		auto pcp{ (char*)p };
		for (size_t i = 0; i < sz; ++i) {
			cout << pcp[i];
			BT->Write(pcp[i]);
		}
		BT->Close();
	}	
*/
	auto pcp{ (char*)p };
	for (size_t i = 0; i < sz; ++i) OutBuf.push_back(pcp[i]);
	return 0;
}

void Compile(string src,string _dest="") {
	err = false;

	// Directories should be done file by file
	if (IsDir(src)) {
		cout << "Analysing directory: " << src << endl;
		auto FL{ GetTree(src) };
		for (auto sf : FL) {
			if (Lower(ExtractExt(sf)) == "lua") Compile(src + "/" + sf, StripExt(sf) + ".lbc");
		}
		return;
	}

	if (!IsFile(src)) { Error("File or directory not found (" + src + ")"); return; }
	cout << "Compiling: " << src << endl;
	auto dest{ StripExt(src) + ".lbc" }; // lbc = Lua Byte Code
	if (_dest.size()) dest = _dest;
	auto L{ luaL_newstate() };
	auto source{ LoadString(src) };
	luaL_openlibs(L);
	lua_atpanic(L, LuaPaniek);
	luaL_loadstring(L, source.c_str());
	//lua_call(L, 0, 0);
	if (!err) {
		OutBuf.clear();
		lua_dump(L, DumpLua, NULL, 0);
	}
	if (Out) {
		cout << "Bundling:  " << dest << endl;
		Out->AddCharacters(dest, OutBuf);
	} else {
		cout << "Writing:   " << dest << endl;
		auto BT{ WriteFile(dest) };
		for (auto ch : OutBuf) BT->Write(ch);
		BT->Close();
	}
	lua_close(L);
}
#pragma endregion


#pragma region Main
void Explain() {
	cout << "Quick program to compile and/or bundle Lua scripts\n\n";
	cout << "Syntax: " << StripAll(Args.myexe) << " [flags] <files>\n\n";
	cout << "-b <Bundle>    Bundle all compiled scripts in a JCR file\n\n";
}

int main(int c, char** _args) {
	cout << "JLCB\nCoded by: Jeroen P. Broks\nBuilt: "<<__DATE__<<"; "<<__TIME__"\n(c) Jeroen P. Broks 2022-"<<TrickyUnits::right(__DATE__,4)<<"\n\n\n";
	init_JCR6();
	AddFlag_String(Flags, "b", "");
	Args = ParseArg(c, _args, Flags);
	if (ParseArgReport().size()) Error(ParseArgReport());
	if (!Args.arguments.size()) {
		Explain(); 
		return 0;
	}
	OutBundle = Args.string_flags["b"];
	if (OutBundle.size()) {
		cout << "Creating bundle: " << OutBundle << "\n";
		Out = make_shared<JT_Create>(OutBundle);
	}
	for (auto f : Args.arguments) { Compile(f); }
	if (Out) {
		cout << "Finalizing bundle\n";
		Out->Close();
	}
	return 0;
}
#pragma endregion