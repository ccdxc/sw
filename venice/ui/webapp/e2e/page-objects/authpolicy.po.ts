import { browser, by, element, protractor, ElementFinder } from 'protractor';

export class AuthPolicy {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/settings/authpolicy');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const autopolicyPageComponent = element(by.css('app-authpolicy'));
        const urlEC = EC.urlContains('/settings/authpolicy');
        const componentEC = EC.presenceOf(autopolicyPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }

    /** Button for starting config LDAP Authentication */
    getCreateLDAPButton() {
        return element(by.css('.global-button-primary.authpolicy-button.ldap-create'));
    }

    getLDAPConfigBindDN() {
        return element(by.css('.ldap-input.ui-inputtext[formcontrolname="bind-dn"]'));
    }

    getLDAPConfigBaseDN() {
        return element(by.css('.ldap-input.ui-inputtext[formcontrolname="base-dn"]'));
    }

    getLDAPConfigBasepassword() {
        return element(by.css('.ldap-input.ui-inputtext[formcontrolname="bind-password"]'));
    }

    getLDAPServerURLInput() {
        return element(by.css('.ldap-input.ui-inputtext[formcontrolname="url"]'));
    }

    getLDAPServerNameInput() {
        return element(by.css('.ldap-input.ui-inputtext[formcontrolname="server-name"]'));
    }

    getLDAPServerCertInput() {
        return element(by.css('.ldap-server-certs.ui-inputtext[formcontrolname="trusted-certs"]'));
    }

    getLDAPConfigData() {
        const rows = element.all(by.css('.ldap-fieldtitle.ldap-input'));
        return rows;
    }

    getLDAPTestBindButton () {
       return  element.all(by.css('.global-button-primary.ldap-bind-connection-button'));
    }

    getLDAPTestConnectionButton() {
        return  element.all(by.css('.global-button-primary.ldap-test-connection-button'));
    }


    /** Button for starting config RADIUS Authentication */
    getCreateRadiusButton() {
        return element(by.css('.global-button-primary.authpolicy-button.radius-create'));
    }

    getRadiusNASIdInput() {
        return element(by.css('.radius-input.ui-inputtext[formcontrolname="nas-id"]'));
    }

    getRadiusServerURLInput() {
        return element(by.css('.radius-input.ui-inputtext[formcontrolname="url"]'));
    }

    getRadiusServerSecretInput() {
        return element(by.css('.radius-input.ui-inputtext[formcontrolname="secret"]'));
    }

    getRadiusConfigData() {
        const rows = element.all(by.css('.radius-fieldtitle.radius-input'));
        return rows;
    }

    /***********  */

    async openCreateRadisuConfigPanel() {
        const EC = protractor.ExpectedConditions;
        await browser.wait(EC.elementToBeClickable(this.getCreateRadiusButton()), 10000, 'Create-RADIUS button never became clickable');
        await this.getCreateRadiusButton().click();
    }

    async openCreateLDAPConfigPanel() {
        const EC = protractor.ExpectedConditions;
        await browser.wait(EC.elementToBeClickable(this.getCreateLDAPButton()), 10000, 'Create-LDAP button never became clickable');
        await this.getCreateLDAPButton().click();
    }

    async openEditLDAPConfigPanel() {
        const EC = protractor.ExpectedConditions;
        const ldapTitleElement = await element(by.css('.ldap-title'));
        await ldapTitleElement.click();
        const editLdapButton = await element(by.css('.ldap-edit-icon'));
        expect(editLdapButton.isPresent()).toBeTruthy();
        await editLdapButton.click();
    }

    getCreateRADIUSAuthenticationeButton() {
        const radiusCreateButton = element(by.css('.global-button-primary.authpolicy-button.radius-create'));
        return radiusCreateButton;
    }

    getCreateLDAPAuthenticationeButton() {
        const ldapCreateButton = element(by.css('.global-button-primary.authpolicy-button.ldap-create'));
        return ldapCreateButton;
    }

    getConfigsRankArrows() {
        return element(by.css('.authpolicy-rank'));
    }

    getConfigTitle() {
        return element.all(by.css('.authpolicy-config-title')).getText();
    }

    async addRADIUSConfig() {
        // open configure RADIUS panel
        await this.openCreateRadisuConfigPanel();

        await browser.wait(element(by.css('.global-button-primary.authpolicy-save-radius')).isPresent(), 5000);
        const saveRadiusButton = await element(by.css('.global-button-primary.authpolicy-save-radius'));
        expect(saveRadiusButton.getText()).toContain('SAVE');

        await this.getRadiusNASIdInput().sendKeys('nas-id');
        await this.getRadiusServerURLInput().sendKeys('10.11.100.101:1812');
        await this.getRadiusServerSecretInput().sendKeys('testing123');
        saveRadiusButton.click();

        const url = await browser.getCurrentUrl();
        const EC = protractor.ExpectedConditions;
        expect(EC.urlContains('login'));
    }

    async move2ndConfigUp() {
        const oldConfigTitles = await this.getConfigTitle();
        const downIcons = await element.all(by.css('.authpolicy-arrow-down .mat-icon'));
        const the1stDownIcon = downIcons[0];
        await the1stDownIcon.click();
        const toolbarSaveButton = await element(by.css('.toolbar-button.global-button-primary.authpolicy-toolbar-button-save'));
        toolbarSaveButton.click();
        const url = await browser.getCurrentUrl();
        const EC = protractor.ExpectedConditions;
        expect(EC.urlContains('login'));
    }

    async addLDAPConfig() {
        await this.openCreateLDAPConfigPanel();

        await browser.wait(element(by.css('.global-button-primary.authpolicy-save-lap')).isPresent(), 5000);
        const saveLdapButton = await element(by.css('.global-button-primary.authpolicy-save-ldap'));
        expect(saveLdapButton.getText()).toContain('SAVE');

        await this.getLDAPConfigBindDN().sendKeys('pensando\\Administrator');
        await this.getLDAPConfigBaseDN().sendKeys('DC=pensando,DC=io');
        await this.getLDAPConfigBasepassword().sendKeys('G0pensando');
        await this.getLDAPServerURLInput().sendKeys('10.11.100.100:389');
        await this.getLDAPServerNameInput().sendKeys('WIN-HQEM46VDNCA.pensando.io');
        const cert = '-----BEGIN CERTIFICATE-----\nMIIGJDCCBQygAwIBAgITFwAAAAKQ/sQfa/q6fgAAAAAAAjANBgkqhkiG9w0BAQsF\nADBUMRIwEAYKCZImiZPyLGQBGRYCaW8xGDAWBgoJkiaJk/IsZAEZFghwZW5zYW5k\nbzEkMCIGA1UEAxMbcGVuc2FuZG8tV0lOLUhRRU00NlZETkNBLUNBMB4XDTE4MDky\nNzAxMzMxMVoXDTE5MDkyNzAxMzMxMVowJjEkMCIGA1UEAxMbV0lOLUhRRU00NlZE\nTkNBLnBlbnNhbmRvLmlvMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\nirNnLZWxiMlsgv0qmiZa45MpvQsogB6cqQgMhwgJ9jN8bV0zCqFmNXSmUGCInbWD\nJ7sjqtxu/PFlAYn0rssArdVUCYC7ZgjKF+j7iKdxH3+z0KgOyXjiJhljRDNWVHJ0\ndjTxYTELKThAs7d4ePbqL6l09h6Akr2jlYdzA3jlEciHk1NC9fKyvRF8Glajq7VB\nACRTtL4ewS2MvWfM/oQPZFrAqNp8q/CggTdhnd//LhZRnOqrTFZ/q0cohRDBkoGf\nb/L3ysFqE4YFlOnTVbJhpOCNQ+u190GQaaSjlzO0YYtU3YA6hndFcLb14OTuX+WE\nDEu81ZB4kxikba7+fi9gwwIDAQABo4IDGzCCAxcwLwYJKwYBBAGCNxQCBCIeIABE\nAG8AbQBhAGkAbgBDAG8AbgB0AHIAbwBsAGwAZQByMB0GA1UdJQQWMBQGCCsGAQUF\nBwMCBggrBgEFBQcDATAOBgNVHQ8BAf8EBAMCBaAweAYJKoZIhvcNAQkPBGswaTAO\nBggqhkiG9w0DAgICAIAwDgYIKoZIhvcNAwQCAgCAMAsGCWCGSAFlAwQBKjALBglg\nhkgBZQMEAS0wCwYJYIZIAWUDBAECMAsGCWCGSAFlAwQBBTAHBgUrDgMCBzAKBggq\nhkiG9w0DBzAdBgNVHQ4EFgQUtcY/fEgmJ4jzlmHJcTLn6BCEELgwHwYDVR0jBBgw\nFoAUmSdGJOIIEhdy1GZA7gyzku4HdDswgeEGA1UdHwSB2TCB1jCB06CB0KCBzYaB\nymxkYXA6Ly8vQ049cGVuc2FuZG8tV0lOLUhRRU00NlZETkNBLUNBLENOPVdJTi1I\nUUVNNDZWRE5DQSxDTj1DRFAsQ049UHVibGljJTIwS2V5JTIwU2VydmljZXMsQ049\nU2VydmljZXMsQ049Q29uZmlndXJhdGlvbixEQz1wZW5zYW5kbyxEQz1pbz9jZXJ0\naWZpY2F0ZVJldm9jYXRpb25MaXN0P2Jhc2U/b2JqZWN0Q2xhc3M9Y1JMRGlzdHJp\nYnV0aW9uUG9pbnQwgc0GCCsGAQUFBwEBBIHAMIG9MIG6BggrBgEFBQcwAoaBrWxk\nYXA6Ly8vQ049cGVuc2FuZG8tV0lOLUhRRU00NlZETkNBLUNBLENOPUFJQSxDTj1Q\ndWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0\naW9uLERDPXBlbnNhbmRvLERDPWlvP2NBQ2VydGlmaWNhdGU/YmFzZT9vYmplY3RD\nbGFzcz1jZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MEcGA1UdEQRAMD6gHwYJKwYBBAGC\nNxkBoBIEEJJg/oR1v2JMiJUKjFOlJGKCG1dJTi1IUUVNNDZWRE5DQS5wZW5zYW5k\nby5pbzANBgkqhkiG9w0BAQsFAAOCAQEAAmBDGs6aH67hhmZ1jKh4kwkMMsNVVjxC\nKsig9B5YUEyOkOhAuvf41X2weZpQwwiF3BTQ96xaKrSutmVOpdyEVjuoCy9oteEb\ncvjbnlpHDzk8nlwkT7oDEvVH6uNrIQ6b8hpo1rtS23JR+Fj2s+bZuk/J/WmggZYh\nqzBI7uKi7rxkuDy0ejJvk7Beek/vF7qaar/bRdJQiMG/u5VmJPNCisXb9Qr8h0lI\nxx3mMeRQ4B9dAtanwCIkRntp1pmTNuVXD4E/nBjIFGYY/uCDWmDuHGmG1xL26uhX\nIEAsHuBf/zLR4c0QUBgs460vCgpxekUgY0XeXH9ldXqR60bxPF4UJQ==\n-----END CERTIFICATE-----';

        await this.getLDAPServerCertInput().sendKeys(cert);
        saveLdapButton.click();

        const url = await browser.getCurrentUrl();
        const EC = protractor.ExpectedConditions;
        expect(EC.urlContains('login'));
    }

    async testLDAPBindConnection() {
       await this.openEditLDAPConfigPanel();

       await this.getLDAPTestBindButton().click();
       await this.getLDAPTestConnectionButton().click();
       expect(element(by.css('.ldap-test-error-icon.ldap-test-error-icon-bind')).isPresent()).toBeFalsy();
       expect(element(by.css('.ldap-test-error-icon.ldap-test-error-icon-connection')).isPresent()).toBeFalsy();

    }
}
