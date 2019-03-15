import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { IMonitoringFlowExportPolicy, MonitoringFlowExportPolicy } from '@sdk/v1/models/generated/monitoring';
import { E2EuiTools } from './E2EuiTools';

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

    async pressAddFlowExportPolicytButton() {
        const saveTechsupportButton = await element(by.css('.toolbar-button.global-button-primary.flowexportpolicy-button'));
        await saveTechsupportButton.click();
    }

    async createFlowExportPolicy(monitoringFlowExportPolicy: IMonitoringFlowExportPolicy) {
        await this.pressAddFlowExportPolicytButton();
        await browser.sleep(2000);
        // set name
        await E2EuiTools.setInputBoxValue('.newflowexportpolicy-container .newflowexportpolicy-name.ui-inputtext', monitoringFlowExportPolicy.meta.name);
        const syslogInputCSS = 'app-newflowexportpolicy .newflowexportpolicy-container .matchrule-syslog .matchrule-input';

        // syslogInputCSS should show 8 input-boxes, we are preparing values for them
        const values = [
            // source
            monitoringFlowExportPolicy.spec['match-rules'][0].source.endpoints[0],
            monitoringFlowExportPolicy.spec['match-rules'][0].source['ip-addresses'][0],
            monitoringFlowExportPolicy.spec['match-rules'][0].source['mac-addresses'][0],
            // destination
            monitoringFlowExportPolicy.spec['match-rules'][0].destination.endpoints[0],
            monitoringFlowExportPolicy.spec['match-rules'][0].destination['ip-addresses'][0],
            monitoringFlowExportPolicy.spec['match-rules'][0].destination['mac-addresses'][0],

            monitoringFlowExportPolicy.spec['match-rules'][0]['app-protocol-selectors'].applications[0],
            monitoringFlowExportPolicy.spec['match-rules'][0]['app-protocol-selectors'].ports[0]
        ];
        // set values to syslog-inputs.
        await E2EuiTools.setMultipleInputBoxValues(syslogInputCSS, values);

        // set export destination
        await E2EuiTools.setInputBoxValue('app-newflowexportpolicy .newflowexportpolicy-container .syslog-input.ui-inputtext[formcontrolname="destination"', monitoringFlowExportPolicy.spec.exports[0].destination);
        // set export transport
        await E2EuiTools.setInputBoxValue('app-newflowexportpolicy .newflowexportpolicy-container .syslog-input.ui-inputtext[formcontrolname="transport"', monitoringFlowExportPolicy.spec.exports[0].transport);

        // click save button
        await E2EuiTools.clickElement('.toolbar-button.global-button-primary.flowexportpolicy-button');
    }


    async deletelowExportPolicy(monitoringFlowExportPolicy: IMonitoringFlowExportPolicy) {
        const tsTableRowActionDeleteButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringFlowExportPolicy.meta.name);
        await E2EuiTools.clickElement(tsTableRowActionDeleteButtonCSS);

        await browser.sleep(5000); // wait for alert pop-up
        await E2EuiTools.clickConfirmAlertFirstButton();
    }
}
