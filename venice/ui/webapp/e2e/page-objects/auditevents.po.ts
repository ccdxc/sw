import { browser, by, element, protractor, ElementFinder } from 'protractor';

export class Auditevents {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/monitoring/auditevents');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const auditeventsPageComponent = element(by.css('app-auditevents'));
        const urlEC = EC.urlContains('/monitoring/auditevents');
        const componentEC = EC.presenceOf(auditeventsPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }


}
