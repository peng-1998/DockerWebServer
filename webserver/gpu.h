#ifndef GPU_H
#define GPU_H
#include <QString>

class GPU
{
public:
    GPU();
    GPU(const QString &type, int gpuid, double max_memory, double used_memory);

    QString type() const;


    int gpuid() const;


    int max_memory() const;


    int used_memory() const;


private:
    QString _type;
    int _gpuid;
    double _max_memory;
    double _used_memory;
};

#endif // GPU_H
