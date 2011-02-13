////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2011 by Four Factor Studios
//                         All Rights Reserved
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//		Four Factor Studios				(C) 2011
//
//		rev 0. Michael Antonelli		12-FEB-2011
//
//		FFS_Spirit is a pathfinding engine implementation. 
//
////////////////////////////////////////////////////////////////////////////////
#include "FFS_Spirit.h"
#include <Sdl.h>

void		(*LogCallBack)(char* pMsg, int Len);
void*		pPathData;
SDL_mutex*	HostMutex;
SDL_cond*	HostCond;
void*		pObjects;

bool		FFS_Spirit_Init(void* sPathData,SDL_mutex* sMutex, SDL_cond* sCond, void* spObj,void (*sLogCallBack)(char* pMsg,int Len))
{

}