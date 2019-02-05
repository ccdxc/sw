import { browser, by, element, protractor, ElementFinder } from 'protractor';

export class SecurityApps {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/security/securityapps');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const alertseventsPageComponent = element(by.css('app-securityapps'));
        const urlEC = EC.urlContains('/security/securityapps');
        const componentEC = EC.presenceOf(alertseventsPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }

}
