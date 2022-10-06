-- <License Block>
-- ***********************************************************
-- Test/F.lua
-- This particular file has been released in the public domain
-- and is therefore free of any restriction. You are allowed
-- to credit me as the original author, but this is not
-- required.
-- This file was setup/modified in:
-- 2022
-- If the law of your country does not support the concept
-- of a product being released in the public domain, while
-- the original author is still alive, or if his death was
-- not longer than 70 years ago, you can deem this file
-- "(c) Jeroen Broks - licensed under the CC0 License",
-- with basically comes down to the same lack of
-- restriction the public domain offers. (YAY!)
-- ***********************************************************
-- Version 22.10.06
-- </License Block>
function L(a)
	if (a<2) then return 1 end
	return L(a-1)*a
end


for i=1,10 do 
	print(i,L(i))
end