#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

#include "didkit.h"

int main() {
    const char *version = didkit_get_version();
    assert(version != NULL);
    assert(strlen(version) > 0);

    // Trigger error
    const char *vp = didkit_vc_issue_presentation("{}", "{}", "{}");
    assert(vp == NULL);
    const char *error_msg = didkit_error_message();
    assert(error_msg != NULL);
    assert(strlen(error_msg) > 0);
    int error_code = didkit_error_code();
    assert(error_code != 0);

    // Generate key
    const char *key = didkit_vc_generate_ed25519_key();
    if (key == NULL) errx(1, "generate key: %s", didkit_error_message());

    // Get did:key for key
    const char *did = didkit_key_to_did(key);
    if (key == NULL) errx(1, "key to did: %s", didkit_error_message());

    // Issue Credential
    const char *credential = "{"
        "   \"@context\": \"https://www.w3.org/2018/credentials/v1\","
        "   \"id\": \"http://example.org/credentials/3731\","
        "   \"type\": [\"VerifiableCredential\"],"
        "   \"issuer\": \"did:example:30e07a529f32d234f6181736bd3\","
        "   \"issuanceDate\": \"2020-08-19T21:41:50Z\","
        "   \"credentialSubject\": {"
        "       \"id\": \"did:example:d23dd687a7dc6787646f2eb98d0\""
        "   }"
        "}";
    char vc_options[0x1000];
    snprintf(vc_options, sizeof vc_options, "{"
            "  \"type\":\"Ed25519VerificationKey2018\","
            "  \"proofPurpose\": \"assertionMethod\","
            "  \"verificationMethod\": \"%s\""
            "}", did);
    const char *vc = didkit_vc_issue_credential(credential, vc_options, key);
    if (vc == NULL) errx(1, "issue credential: %s", didkit_error_message());

    // Verify Credential
    const char *vc_verify_options = "{\"proofPurpose\": \"assertionMethod\"}";
    const char *res = didkit_vc_verify_credential(vc, vc_verify_options);
    if (res == NULL) errx(1, "verify credential: %s", didkit_error_message());
    if (strstr(res, "\"errors\":[]") == NULL) errx(1, "verify credential result: %s", res);
    didkit_free_string(res);

    // Issue Presentation
    char presentation[0x1000];
    snprintf(presentation, sizeof presentation, "{"
        "   \"@context\": [\"https://www.w3.org/2018/credentials/v1\"],"
        "   \"id\": \"http://example.org/presentations/3731\","
        "   \"type\": [\"VerifiablePresentation\"],"
        "   \"issuer\": \"did:example:30e07a529f32d234f6181736bd3\","
        "   \"issuanceDate\": \"2020-10-19T11:41:50Z\","
        "   \"verifiableCredential\": %s"
        "}", vc);
    char vp_options[0x1000];
    snprintf(vp_options, sizeof vp_options, "{"
            "  \"type\":\"Ed25519VerificationKey2018\","
            "  \"proofPurpose\": \"authentication\","
            "  \"verificationMethod\": \"%s\""
            "}", did);
    vp = didkit_vc_issue_presentation(presentation, vp_options, key);
    if (vp == NULL) errx(1, "issue presentation: %s", didkit_error_message());

    // Verify Presentation
    const char *vp_verify_options = "{\"proofPurpose\": \"authentication\"}";
    res = didkit_vc_verify_presentation(vp, vp_verify_options);
    if (res == NULL) errx(1, "verify presentation: %s", didkit_error_message());
    if (strstr(res, "\"errors\":[]") == NULL) errx(1, "verify presentation result: %s", res);
    didkit_free_string(res);

    didkit_free_string(vp);
    didkit_free_string(vc);
    didkit_free_string(did);
    didkit_free_string(key);
}
