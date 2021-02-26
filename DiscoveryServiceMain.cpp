//===============================================================================
// Copyright (c) Deere & Company as an unpublished work
// THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF DEERE & COMPANY
// ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY
// AUTHORIZED BY DEERE & COMPANY IS PROHIBITED
//===============================================================================

#include "DiscoveryService.h"
#include "ParameterIO/AppArgsChecker.h"

//-------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   if(!ParameterIO::CheckArgsAndInitG3Logger(argc, argv))
   {
      return -1;
   };
   std::atomic<bool> killed(false);
   DiscoveryService service(argv[1], killed);
   return service.Run();
}

//===============================================================================
// Copyright (c) Deere & Company as an unpublished work
// THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF DEERE & COMPANY
// ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY
// AUTHORIZED BY DEERE & COMPANY IS PROHIBITED
//===============================================================================
