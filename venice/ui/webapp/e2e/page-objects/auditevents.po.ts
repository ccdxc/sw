import { browser, by, element, protractor, ElementFinder } from 'protractor';

export class Auditevents {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/monitoring/auditevents');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const alertseventsPageComponent = element(by.css('app-auditevents'));
        const urlEC = EC.urlContains('/monitoring/auditevents');
        const componentEC = EC.presenceOf(alertseventsPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }

    

}
