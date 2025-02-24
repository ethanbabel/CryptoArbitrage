#ifndef SES_H
#define SES_H

#include <aws/core/Aws.h>
#include <aws/sesv2/SESV2Client.h>
#include <aws/sesv2/model/SendEmailRequest.h>
#include <memory>

class SESEmailer {
private:
    Aws::SDKOptions options;  
    std::shared_ptr<Aws::SESV2::SESV2Client> ses_client;

public:
    SESEmailer(); 
    ~SESEmailer();
    
    void sendEmail(const std::string& to, const std::string& subject, const std::string& body);
};

#endif