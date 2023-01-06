#ifndef SOCKET_READ_WRITE_H
#define SOCKET_READ_WRITE_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QBuffer>
#include <QByteArray>
class BlockReader
{
public:
    BlockReader(QIODevice *io)
    {
        buffer.open(QIODevice::ReadWrite);
        _stream.setVersion(QDataStream::Qt_6_4);
        _stream.setDevice(&buffer);

        quint64 blockSize;

        // Read the size.
        readMax(io, sizeof(blockSize));
        buffer.seek(0);
        _stream >> blockSize;

        // Read the rest of the data.
        readMax(io, blockSize);
        buffer.seek(sizeof(blockSize));
    }

    QDataStream& stream()
    {
        return _stream;
    }

private:
    // Blocking reads data from socket until buffer size becomes exactly n. No
    // additional data is read from the socket.
    void readMax(QIODevice *io, int n)
    {
        n = n + buffer.size();
        while (buffer.size() < n) {
            if (!io->bytesAvailable()) {
                io->waitForReadyRead(10000);
            }
            buffer.write(io->read(n - buffer.size()));
        }
    }
    QBuffer buffer;
    QDataStream _stream;
};

class BlockWriter
{
public:
    BlockWriter(QIODevice *io)
    {
        buffer.open(QIODevice::WriteOnly);
        this->io = io;
        _stream.setVersion(QDataStream::Qt_6_4);
        _stream.setDevice(&buffer);

        // Placeholder for the size. We will get the value
        // at the end.
        _stream << quint64(0);
    }

    ~BlockWriter()
    {
        // Write the real size.
        _stream.device()->seek(0);
        _stream << (quint64) buffer.size();

        // Flush to the device.
        io->write(buffer.buffer());
    }

    QDataStream &stream()
    {
        return _stream;
    }

private:
    QBuffer buffer;
    QDataStream _stream;
    QIODevice *io;
};
#endif // SOCKET_READ_WRITE_H
