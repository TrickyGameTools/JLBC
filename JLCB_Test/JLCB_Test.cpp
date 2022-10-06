// Lic:
// JCR Lua Compile & Bundle
// Testing Utility
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
#include <iostream>

#include <lua.hpp>

#include <QuickTypes.hpp>
#include <QuickString.hpp>
#include <QuickStream.hpp>
#include <ArgParse.h>

#include <jcr6_core.hpp>

using namespace std;
using namespace TrickyUnits;
using namespace jcr6;

FlagConfig Flags{};
ParsedArg Args{};
string InBundle{};
JT_Dir J{};

#pragma region Errors
uint64 Errors{ 0 };
void Error(string msg, bool fatal = false) {
	printf("\x1b[31mERROR #%05llu:\x1b[0m  %s\n", ++Errors, msg.c_str());
	if (fatal) exit(1);
}
#pragma endregion

#pragma region Execution

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
	exit(11);
	err = true;
	return 0;
}

char* ldata{ nullptr };
size_t lpos{ 0 };
char* rdata{ nullptr };
/*

const char* Reader(lua_State* L, void* data, size_t* size) {
	auto sz = sizeof(data);
	cout << "DEBUG Reader( <lua_State>" << (int)L << ", <data>\"" << (char*)data << "\", " << size << ") -- lpos{" << lpos << "}; sz{" << sz << "};\n"; // DEBUG ONLY
	*size = sz;
	if (rdata) { delete[] rdata; rdata = nullptr; }
	if (lpos > sizeof(ldata)) {
		if (rdata) { delete[] rdata; }
		rdata = nullptr;
		return nullptr;
	}
	rdata = new char[*size];
	for (size_t i = 0; i < *size; i++) {
		rdata[i] = ldata[lpos++];
		cout << "DEBUG Ret " << rdata << endl;
	}
	return rdata;
}
*/


void Execute(string f) {
	err = false;
	cout << "Executing: " << f << endl;
	uint64 size{ 0 };
	if (InBundle.size()) {
		if (!J.EntryExists(f)) { Error("Bundle has no entry named: " + f); return; }
		//auto size{ J.Entry(f).RealSize() };
		size = J.Entry(f).RealSize();
		if (!size) { Error("No data in entry"); return; }
		ldata = new char[size];
		auto BT = J.B(f);
		for (int i = 0; i < size; ++i) ldata[i] = BT->ReadChar();
	} else {
		if (!FileExists(f)) { Error("File not found: " + f); return; }
		//auto size{ FileSize(f) };
		size = FileSize(f);
		if (!size) { Error("No data in file"); return; }
		ldata = new char[size];
		auto BT = ReadFile(f);
		for (int i = 0; i < size; ++i) ldata[i] = BT->ReadChar();
		BT->Close();
	}
	if (!ldata) { Error("Loaded data is NULL"); return; }
	auto L{ luaL_newstate() };
	lpos = 0;
	lua_atpanic(L, LuaPaniek);
	luaL_openlibs(L);
	//lua_load(L, Reader, (void*)ldata, f.c_str(), NULL);
	//cout << "Buffer loaded: " << size << endl;
	luaL_loadbuffer(L, ldata, size, f.c_str());
	lua_call(L, 0, 0);
	//if (!err) {
	lua_close(L);
	//}

	delete[] ldata;
	if (rdata) {
		delete[]  rdata;
		rdata = nullptr;
	}
}

void Explain() {
	cout << "Quick program to test the results of JLCB\n\n";
	cout << "Syntax: " << StripAll(Args.myexe) << " [flags] <files>\n\n";
	cout << "-b <Bundle>    Get all (pre-compiled) scripts from a JCR file\n\n";
	cout << "<PLEASE NOTE>\nThis is only a test utility and for that reason I did NOT include support for zlib or any other compression algorithms as that was not what required any testing!\n\n";
}
#pragma endregion


int main(int c, char** _args) {
	cout << "JLCB Test Utility\nCoded by: Jeroen P. Broks\nBuilt: " << __DATE__ << "; " << __TIME__"\n(c) Jeroen P. Broks 2022-" << TrickyUnits::right(__DATE__, 4) << "\n\n\n";
	init_JCR6();
	AddFlag_String(Flags, "b", "");
	Args = ParseArg(c, _args, Flags);
	if (ParseArgReport().size()) Error(ParseArgReport());
	if (!Args.arguments.size()) {
		Explain();
		return 0;
	}
	InBundle = Args.string_flags["b"];

	if (InBundle.size()) {
		cout << "Reading bundle: " << InBundle << endl;
		J = Dir(InBundle);
		if (Upper(JCR_Error) != "OK") { Error("Bundle cound not be opened!\nJCR6 reported -> " + JCR_Error, true); }
	}
	for (auto f : Args.arguments) Execute(f);
	return 0;
}