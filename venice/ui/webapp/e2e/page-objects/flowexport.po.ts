import { browser, by, element, protractor, ElementFinder } from 'protractor';

export class FlowExport {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/monitoring/flowexport');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const flowExportPageComponent = element(by.css('app-flowexport'));
        const urlEC = EC.urlContains('/monitoring/flowexport');
        const componentEC = EC.presenceOf(flowExportPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }


}
