import { browser, by, element, protractor, ElementFinder } from 'protractor';

export class TechSupport {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/monitoring/techsupport');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const techsupportPageComponent = element(by.css('app-techsupport'));
        const urlEC = EC.urlContains('/monitoring/techsupport');
        const componentEC = EC.presenceOf(techsupportPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }
}
