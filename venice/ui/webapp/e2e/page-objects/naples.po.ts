import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { E2EuiTools } from './E2EuiTools';

export class Naples {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/cluster/dscs');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const naplesPageComponent = element(by.css('app-naples'));
        const urlEC = EC.urlContains('/cluster/dscs');
        const componentEC = EC.presenceOf(naplesPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }
}
