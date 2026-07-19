/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ERRORS_H
#define ERRORS_H

/**
 * Enumeration of all possible error codes.
 */
enum ErrorCodes
  {
  ENoError = 0,
  /* Insert errors below. */
  /* Global errors 1-99 */
  EWrongDriverCPUVersion,
  ELibraryBaseInconsistency,
  EOpenDosBase,
  EOpenIntuitionBase,
  EOpenExpansionBase,
  EOpenLogFile,
  EAllocateLogMem,
  EHandleUnknown,
  EAllocateInterrupt,
  EAllocateHandle,
  EAllocateBuffer,
  EDriverInUse,
  EDriverNotInUse,

  /* Missing implementation 100-199*/
  ESomethingNotImplemented = 100,  

  /* Hardware errors 200-300 */
  EAmiGUSNotFound = 200,
  EAmiGUSDetectError,
  EAmiGUSFirmwareOutdated,

  /* Insert errors above. */
  EUnknownError,
  EAmountOfErrors
  };

#endif /* ERRORS_H */
