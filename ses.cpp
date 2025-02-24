#include "ses.h"
#include <aws/core/utils/memory/stl/AWSString.h>
#include <iostream>

#define FROM_EMAIL "ebabelCryptoArb@gmail.com"  // Always use this sender email

// Constructor - Initializes AWS SDK and SES Client
SESEmailer::SESEmailer() {
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;

    Aws::InitAPI(options);  // Initialize SDK

    Aws::Client::ClientConfiguration config;
    config.region = "us-west-1";  // Set AWS region (modify if needed)

    ses_client = std::make_shared<Aws::SESV2::SESV2Client>(config);
}

// Destructor - Shuts down AWS SDK
SESEmailer::~SESEmailer() {
    Aws::ShutdownAPI(options);
}

// Sends an email using AWS SESv2
void SESEmailer::sendEmail(const std::string& to, const std::string& subject, const std::string& body) {
    Aws::SESV2::Model::SendEmailRequest request;
    request.SetFromEmailAddress(FROM_EMAIL);  // Use fixed sender email

    // Set recipient
    Aws::SESV2::Model::Destination destination;
    destination.AddToAddresses(Aws::String(to.c_str()));
    request.SetDestination(destination);

    // Set message content
    Aws::SESV2::Model::EmailContent content;
    Aws::SESV2::Model::Message message;
    Aws::SESV2::Model::Content subjectContent, bodyContent;

    subjectContent.SetData(Aws::String(subject.c_str()));
    bodyContent.SetData(Aws::String(body.c_str()));

    Aws::SESV2::Model::Body emailBody;
    emailBody.SetText(bodyContent);
    message.SetBody(emailBody);
    message.SetSubject(subjectContent);

    content.SetSimple(message);
    request.SetContent(content);

    // Send the email
    auto outcome = ses_client->SendEmail(request);
    if (!outcome.IsSuccess()) {
        std::cerr << "Failed to send email: " 
                  << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Email sent successfully!" << std::endl;
    }
}