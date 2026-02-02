# Security Policy

This software is verified for security for official releases and as such does
not make promises about the quality of the product for patches delivered between
releases.

## Trust model for VGF inputs

VGF files (including embedded SPIR-V™) are expected to be provided by trusted
tooling in the build and deployment pipeline. The decoder assumes VGF content is
trusted input and does not attempt to sanitize or sandbox SPIR-V™ payloads.
Safeguards to improve robustness are in place, but they do not change the trust
boundary. Consumers that accept VGF from untrusted sources must implement their
own validation/sandboxing at the application boundary. Callers must perform VGF
header decoding to check section offsets and sizes against the file length
before any decoder is constructed.

## Reporting a Vulnerability

Security vulnerabilities may be reported to the Arm® Product Security Incident
Response Team (PSIRT) by sending an email to
[psirt@arm.com](mailto:psirt@arm.com).

For more information visit
<https://developer.arm.com/support/arm-security-updates/report-security-vulnerabilities>
