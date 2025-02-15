/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
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
  /* Hardware errors 200-300 */
  EAmiGUSNotFound = 200,
  EAmiGUSDetectError,
  EAmiGUSFirmwareOutdated,
  EAmiGUSWriteFailed,
  EAmiGUSReadFailed,
  EAmiGUSResetFailed,
  /* Insert errors above. */
  EUnknownError,
  EAmountOfErrors
  };

#endif /* ERRORS_H */
