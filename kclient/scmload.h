#ifndef __SCMLOAD_H__
#define __SCMLOAD_H__

#pragma once

#include <Windows.h>


SC_HANDLE ScmOpenSCMHandle();

VOID ScmCloseSCMHandle(SC_HANDLE hscm);
// ������� ��������� �������� �� ������ SCM �������
BOOL ScmInstallDriver( SC_HANDLE  scm, LPCTSTR DriverName, LPCTSTR driverExec );
// ������� �������� �������� �� ������ SCM �������
BOOL ScmRemoveDriver(SC_HANDLE scm, LPCTSTR DriverName);
// ������� ������� �������� �� ������ SCM �������
BOOL ScmStartDriver(SC_HANDLE  scm, LPCTSTR DriverName);
// ������� �������� �������� �� ������ SCM �������
BOOL ScmStopDriver(SC_HANDLE  scm, LPCTSTR DriverName);


#endif
