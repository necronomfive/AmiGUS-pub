/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ERRORS_H
#define ERRORS_H

/**
 * Enumeration of all possible error codes.
 */
enum TErrorCodes
  {
  ENoError = 0,
  /* Insert errors below. */
  /* Global errors 1-99 */
  EWrongDriverCPUVersion,
  ELibraryBaseInconsistency,
  EOpenDosBase,
  EOpenIntuitionBase,
  EOpenUtilityBase,
  EOpenExpansionBase,
  EAllocateTimerRequest,
  EOpenTimerDevice,
  EOpenLogFile,
  EAllocateLogMem,
  EDriverInUse,
  EDriverNotInUse,
  EOutOfMemory,
  EAllocatePlaybackBuffers,
  EAllocateRecordingBuffers,
  EWorkerProcessCreationFailed,
  EWorkerProcessDied,
  EWorkerProcessSignalsFailed,
  EMainProcessSignalsFailed,
  ERecordingModeNotSupported,
  /* Missing implementation 100-199*/
  EAllocAudioNotImplemented = 100,  
  EFreeAudioNotImplemented,
  EDisableNotImplemented,
  EEnableNotImplemented,
  EStartNotImplemented,
  EUpdateNotImplemented,
  EStopNotImplemented,
  ESetVolNotImplemented,
  ESetFreqNotImplemented,
  ESetSoundNotImplemented,
  ESetEffectNotImplemented,
  ELoadSoundNotImplemented,
  EUnloadSoundNotImplemented,
  EGetAttrNotImplemented,
  EHardwareControlNotImplemented,
  EAudioModeNotImplemented,
  ERecordingNotImplemented,
  ESampleFormatMissingFromMode,
  ECopyFunctionMissingFromMode,
  EShiftMissingFromMode,
  /* Hardware errors 200-300 */
  EAmiGUSNotFound = 200,
  EAmiGUSDetectError,
  EAmiGUSInUseError,
  EAmiGUSFirmwareOutdated,
  EAmiGUSWriteFailed,
  EAmiGUSReadFailed,
  EAmiGUSResetFailed,
  /* Insert errors above. */
  EUnknownError,
  EAmountOfErrors
  };

#endif /* ERRORS_H */
