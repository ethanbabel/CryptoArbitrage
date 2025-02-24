#include "ses.h"

int main() {
    SESEmailer emailer;
    emailer.sendEmail("babelethan@gmail.com", "Test Subject", "Test Body");
    return 0;
}