#include "httprequest.h"

HttpRequest::HttpRequest(TcpSocket *_socket)
    :QObject(),
      header(),
      rawData(),
      formData(),
      hasSetFormData(false),
      totalBytes(0),
      bytesHaveRead(0),
      rawHeader(),
      socket(_socket)
{
}

HttpRequest::HttpRequest(const HttpRequest &in)
    :QObject(),
      header(in.header),
      rawData(in.rawData),
      formData(in.formData),
      hasSetFormData(in.hasSetFormData),
      totalBytes(in.totalBytes),
      bytesHaveRead(in.bytesHaveRead),
      rawHeader(in.rawHeader),
      socket(in.socket)
{
}

void HttpRequest::operator=(const HttpRequest &in)
{
    header=in.header;
    rawData=in.rawData;
    formData=in.formData;
    hasSetFormData=in.hasSetFormData;
    totalBytes=in.totalBytes;
    bytesHaveRead=in.bytesHaveRead;
    rawHeader=in.rawHeader;
    socket=in.socket;
}


HttpRequest::~HttpRequest()
{

}

void HttpRequest::appendData(const char* buffer,unsigned int size)
{
    rawData.append(buffer,size);
    bytesHaveRead+=size;
}

void HttpRequest::appendData(const QByteArray &ba)
{
    rawData.append(ba);
    bytesHaveRead+=ba.count();
}

void HttpRequest::setRawHeader(const QString &_rh)
{
    rawHeader=_rh;
}

void HttpRequest::parseFormData()
{
    QString contentLengthString=header.getHeaderInfo("Content-Length");

    if(!contentLengthString.isEmpty())
    {
        //int contentLength=contentLengthString.toInt();

        QString contentTypeString=header.getHeaderInfo("Content-Type");

        if(!contentTypeString.isEmpty())
        {
            int i=0;
            for(i=0;i<contentTypeString.count();++i)
            {
                if(contentTypeString.at(i)==';')
                    break;
            }

            QString contentType=contentTypeString.mid(0,i);

            if(contentType=="multipart/form-data")
            {
                    bool success=false;

                    QMap<QString,QByteArray> _formData;

                    if(contentTypeString.mid(i+2,9)=="boundary=")
                    {
                        QString boundary=contentTypeString.mid(i+11,contentTypeString.count()-i-11);

                        int linebegin=0;
                        int lineend=0;

                        while(rawData.at(lineend)!='\r' && rawData.at(lineend+1)!='\n')
                        {
                            ++lineend;
                        }

                        QString boundaryCheck= QByteArray(&rawData.data()[linebegin],lineend-linebegin);

                        if(boundaryCheck=="--"+boundary)
                        {
                            lineend+=2;
                            linebegin=lineend;

                            while(lineend<rawData.count())
                            {
                                while(lineend<rawData.count()-1 && rawData.at(lineend)!='\r' && rawData.at(lineend+1)!='\n')
                                {
                                    ++lineend;
                                }

                                QString fieldCheck= QByteArray(&rawData.data()[linebegin],lineend-linebegin);

                                if(!(fieldCheck.left(38)=="Content-Disposition: form-data; name=\""))
                                {
                                    break;
                                }

                                int namelength=38;

                                while(fieldCheck.at(namelength)!='\"' && namelength<fieldCheck.count())
                                {
                                    ++namelength;
                                }

                                QString fieldName=fieldCheck.mid(38,namelength-38);

                                lineend+=2;
                                linebegin=lineend;

                                if(lineend>=rawData.count())
                                    break;

                                while(lineend<rawData.count()-1 && rawData.at(lineend)!='\r' && rawData.at(lineend+1)!='\n')
                                {
                                    ++lineend;
                                }

                                lineend+=2;
                                linebegin=lineend;

                                QByteArray value;

                                while(lineend<rawData.count())
                                {
                                    while(lineend<rawData.count()-1 && rawData.at(lineend)!='\r' && rawData.at(lineend+1)!='\n')
                                    {
                                        ++lineend;
                                    }

                                    QByteArray thisline(&rawData.data()[linebegin],lineend-linebegin);
                                    QString aValueLine=thisline;

                                    if(aValueLine.left(2+boundary.count())=="--"+boundary)
                                    {

                                        _formData[fieldName]=value;
                                        if(aValueLine.right(2)=="--")
                                        {
                                            success=true;
                                        }

                                        lineend+=2;
                                        linebegin=lineend;

                                        break;
                                    }

                                    value.append(thisline);
                                    value.append('\r');
                                    value.append('\n');

                                    lineend+=2;
                                    linebegin=lineend;
                                }
                            }
                        }
                    }



                if(success)
                {
                    formData=(_formData);
                    hasSetFormData=true;
                }
            }

        }

    }
}
