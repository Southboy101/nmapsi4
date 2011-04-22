/***************************************************************************
 *   Copyright (C) 2008-2011 by Francesco Cecconi                          *
 *   francesco.cecconi@gmail.com                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License.        *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "scanThread.h"

scanning::scanThread::scanThread(QByteArray& ProcB1, QByteArray& ProcB2,
                       const QStringList parameters, QObject *parent)
     : pout(ProcB1), 
       perr(ProcB2),
       ParList(parameters),
       proc(NULL),
       par(parent)
{ 
}

scanning::scanThread::~scanThread()
{
    qDebug() << "DEBUG ~scanThread()";
    stopProcess();
}

void scanning::scanThread::run() 
{     
     proc = new QProcess();
     qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

     connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
             this, SLOT(setValue()));
     connect(proc, SIGNAL(readyReadStandardOutput()),
             this, SLOT(realtimeData()));

#ifndef THREAD_NO_DEBUG
     qDebug() << "DEBUG::ThreadString:: " << ParList;
#endif
     proc->start("nmap", ParList);
     
     exec();
     // emit signal, scan is end
     emit upgradePR();
     emit threadEnd(ParList, pout, perr);

#ifndef THREAD_NO_DEBUG
     qDebug() << "scan() THREAD:: Quit";
#endif
}

void scanning::scanThread::setValue() 
{
#ifndef THREAD_NO_DEBUG
     qDebug() << "scan() THREAD:: -> start";
#endif
     // set scan return buffer
     perr  = proc->readAllStandardError(); // read error buffer
     proc->close();
     delete proc;
     exit(0);
}

void scanning::scanThread::stopProcess()
{
     // stop scan process (Slot)
     if (!proc) {
	 return;
     }
     
     if(proc->state() == QProcess::Running) {
#ifndef THREAD_NO_DEBUG
	  qDebug() << "scan() THREAD:: Clear Process";
#endif
	  proc->close();
	  delete proc;
     }
}

void scanning::scanThread::realtimeData() 
{
    // read realtime data from QProcess
    QByteArray realByte = proc->readAllStandardOutput();
    pout.append(realByte);
    QString stream_(realByte);
    // emit signal for data trasmission to parent
    if (!stream_.isEmpty()) {
	emit flowFromThread(ParList[ParList.size()-1], stream_);
    }
}
