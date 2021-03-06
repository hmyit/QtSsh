#include "sshprocess.h"
#include "sshclient.h"
#include <QTimer>
#include <QEventLoop>

Q_LOGGING_CATEGORY(logsshprocess, "ssh.process", QtWarningMsg)

SshProcess::SshProcess(SshClient *client) : SshChannel(client)
{
    sshChannel = qssh2_channel_open(sshClient->session());
    if (sshChannel == nullptr)
    {
        qCDebug(logsshprocess) << "Channel session open failed";
        return;
    }

    qCDebug(logsshprocess) << "Channel session opened";
}

SshProcess::~SshProcess()
{
    stopChannel();
}

QByteArray SshProcess::runCommand(const QString &cmd)
{
    qCDebug(logsshprocess) << "runCommand(" << cmd << ")";
    int ret = qssh2_channel_exec(sshChannel, cmd.toStdString().c_str());
    if (ret)
    {
        qDebug() << "ERROR : QtSshChannel : process exec failed " << ret;
        return QByteArray();
    }

    bool eof = false;
    QByteArray result;
    ssize_t retsz;
    char buffer[16*1024];

    while(!eof)
    {

        retsz = qssh2_channel_read(sshChannel, buffer, 16 * 1024);
        if(retsz < 0)
        {
            qCWarning(logsshprocess) << "ERROR: can't read result (" << retsz << ")";
            return QByteArray();
        }
        result.append(buffer, static_cast<int>(retsz));
            qCDebug(logsshprocess) << "runCommand(" << cmd << ") -> " << result;

        if (qssh2_channel_eof(sshChannel) == 1)
        {
            eof = true;
        }
    }
    qCDebug(logsshprocess) << "runCommand(" << cmd << ") -> " << result;
    return result;
}
