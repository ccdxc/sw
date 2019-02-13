import { browser, by, element, protractor, ElementFinder } from 'protractor';

export class FirewallLogs {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/monitoring/fwlogs');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const fwlogsPageComponent = element(by.css('app-fwlogs'));
        const urlEC = EC.urlContains('/monitoring/fwlogs');
        const componentEC = EC.presenceOf(fwlogsPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }
}
