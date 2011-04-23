# This file is part of HadesMem.
# Copyright (C) 2011 Joshua Boyce (a.k.a. RaptorFactor).
# <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>
# 
# HadesMem is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# HadesMem is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with HadesMem.  If not, see <http://www.gnu.org/licenses/>.

ProcName = raw_input("Process name: ")
MyMem = PyHadesMem.MemoryMgr(ProcName)
ModPath = raw_input("Module name/path: ")
MyMod = PyHadesMem.Module(MyMem, ModPath)
print("Module base address: " + hex(MyMod.GetBase()))
FuncName = raw_input("Function name: ")
FuncRemote = MyMem.GetRemoteProcAddress(MyMod.GetBase(), ModPath, FuncName)
print("Function address: " + hex(FuncRemote))
FuncRet = MyMem.Call(FuncRemote, [], PyHadesMem.MemoryMgr.CallConv.Default)
print("Function return: " + hex(FuncRet[0]) + " " + str(FuncRet[0]))
print("Function last error: " + hex(FuncRet[1]) + " " + str(FuncRet[1]))