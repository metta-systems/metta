#pragma once

//#include <QIODevice>

/*!
 * The block device emulates a disk block device with configured block size and access times. It uses a regular file in
 * a filesystem to store data.
 */
class BlockDevice //: public QIODevice
{
    QString storageFileName;
    QFile storageFile;
    blocksize_t blockSize;
    blockno_t numBlocks;
public:
    BlockDevice(const QString& storageFile, bool create = false, blocksize_t blockSize = 0, blockno_t numBlocks = 0);
    virtual ~BlockDevice();

    /*!
     * blockno_t type designates a block address type, main unit of addressing in a block device.
     */
    typedef unsigned long long blockno_t;
    typedef size_t blocksize_t;

    /*!
     * Return time in nanoseconds it would take to seek from current block position to seekTo.
     */
    int seekTime(blockno_t seekTo);

    void seek(blockno_t block);
    blockno_t pos();

    /*!
     * Read and write functions operate on whole blocks of specific size.
     */
    QByteArray readBlock(blockno_t block);
    void writeBlock(blockno_t block, const QByteArray& buffer);
};


