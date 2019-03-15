import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { E2EuiTools } from './E2EuiTools';

export class Cluster {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/cluster/cluster');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const clusterPageComponent = element(by.css('app-cluster'));
        const urlEC = EC.urlContains('/cluster/cluster');
        const componentEC = EC.presenceOf(clusterPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }
}
