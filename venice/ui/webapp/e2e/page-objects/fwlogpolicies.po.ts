import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { E2EuiTools } from './E2EuiTools';
import { MonitoringFwlogPolicy, IMonitoringFwlogPolicy } from '@sdk/v1/models/generated/monitoring';

export class FwlogPolicies {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/monitoring/fwlogs/fwlogpolicies');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const flowExportPageComponent = element(by.css('app-fwlogpolicies'));
        const urlEC = EC.urlContains('/monitoring/fwlogs/fwlogpolicies');
        const componentEC = EC.presenceOf(flowExportPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }

    async pressAddFwlogPolicytButton() {
        const saveFwlogPolicyButton = await element(by.css('.toolbar-button.global-button-primary.fwlogpolicies-button'));
        await saveFwlogPolicyButton.click();
    }

    async createNewFwlogPolicy(monitoringFwlogPolicy: IMonitoringFwlogPolicy) {
        await this.pressAddFwlogPolicytButton();
        await browser.sleep(2000);

        // set name
        await E2EuiTools.setInputBoxValue('app-newfwlogpolicy .newfwlogpolicy-name.ui-inputtext', monitoringFwlogPolicy.meta.name);

        // set  destination
        await E2EuiTools.setInputBoxValue('app-newfwlogpolicy  .syslog-input.ui-inputtext[formcontrolname="destination"]', monitoringFwlogPolicy.spec.targets[0].destination);


        // set  transport
        await E2EuiTools.setInputBoxValue('app-newfwlogpolicy .syslog-input.ui-inputtext[formcontrolname="transport"]', monitoringFwlogPolicy.spec.targets[0].transport);
        await browser.sleep(2000);
         // click save button
         await E2EuiTools.clickElement('.toolbar-button.global-button-primary.fwlogpolicies-button.fwlogpolicies-button-SAVE');
    }

    async deleteFwlogPolicy(monitoringFwlogPolicy: IMonitoringFwlogPolicy) {
        const tsTableRowActionDeleteButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringFwlogPolicy.meta.name);
        await E2EuiTools.clickElement(tsTableRowActionDeleteButtonCSS);
        await E2EuiTools.clickConfirmAlertFirstButton();
    }


}

