import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { EventsEvent_severity_uihint } from '@sdk/v1/models/generated/events';
import { IMonitoringAlertDestination, IMonitoringAlertPolicy, IApiStatus, MonitoringAlertPolicy, MonitoringAlertDestination } from '@sdk/v1/models/generated/monitoring';
import { E2EuiTools } from './E2EuiTools';
import { AppPage } from './app.po';
import { FieldSelectorCriteria } from './.';

export class Alertsevents {
  configs = {
    alertsevents: {
      url: '/#/monitoring/alertsevents',
      css: 'app-alertsevents'
    }
  };

  constructor() {
  }

  navigateTo(page: string) {
    return browser.get(this.configs[page].url); // '/#/monitoring/alertsevents');
  }

  async verifyPage(page: string) {
    const EC = protractor.ExpectedConditions;
    const alertseventsPageComponent = element(by.css(this.configs[page].css)); // 'app-alertsevents'
    const urlEC = EC.urlContains(this.configs[page].url); // '/monitoring/alertsevents'
    const componentEC = EC.presenceOf(alertseventsPageComponent);
    await browser.wait(EC.and(urlEC, componentEC));
  }

  async verifyTableHasContents() {
    const EC = protractor.ExpectedConditions;
    await browser.wait(element(by.css('.ui-table-scrollable-body-table tbody tr td')).isPresent(), 5000);
    // Let rendering finish
    await browser.sleep(1000);
    const rows = await element.all(by.css('.ui-table-scrollable-body-table tbody tr'));
    let limit = rows.length;
    // Limiting to first 10 events due to the maount of time it takes
    // to check each row
    if (limit > 10) {
      limit = 10;
    }
    for (let index = 0; index < limit; index++) {
      // We re select the element to avoid our reference being stale
      const colVals = await element.all(by.css('.ui-table-scrollable-body-table tbody tr:nth-of-type(' + index + ') td'));
      for (let colIndex = 0; colIndex < colVals.length; colIndex++) {
        const colVal: ElementFinder = colVals[colIndex];
        const colText = await colVal.getText();
        // to debug -- console.log('row: ' + index  + ' column: ' + colIndex + ' value:' + colText);
        if (colIndex === 2) {
          // Severity column
          // Icon should be present
          await browser.wait(EC.presenceOf((colVal.element(by.css('mat-icon')))), 1000, 'mat-icon for severity was missing');
          // Checking severity is a ui hint value
          const severityValues = Object.values(EventsEvent_severity_uihint);
          const ecSeverity: Function[] = [];
          const actualSeverityText = await colVal.element(by.css('div')).getText();
          expect(severityValues).toContain(actualSeverityText, 'severity value was not one of the ui hint values');
        } else {
          if (colIndex < (colVals.length - 1)) {
            expect(colText).not.toBe('');
          } else {
            expect(colText).toBe(''); // last column contains action icons
          }
        }
      }
    }
  }



  async createAlertDestination(monitoringAlertDestination: IMonitoringAlertDestination) {
    await this.openEventAlertPolicyPage();
    // open destinatin tab
    this.openDestionationTab();
    await browser.sleep(5000);  // wait for overlay node dismiss
    // click 'ADD DESTINATION' button
    await E2EuiTools.clickElement('.toolbar-button.global-button-primary.destinations-button');
    await browser.sleep(1000);

    // set name and other fields
    await E2EuiTools.setInputBoxValue('app-newdestination .newdestination-name.ui-inputtext', monitoringAlertDestination.meta.name);

    await E2EuiTools.setInputBoxValue('app-newdestination .syslog-input.ui-inputtext[formcontrolname="destination"]', monitoringAlertDestination.spec['syslog-export'].targets[0].destination);

    await E2EuiTools.setInputBoxValue('app-newdestination .syslog-input.ui-inputtext[formcontrolname="transport"]', monitoringAlertDestination.spec['syslog-export'].targets[0].transport);

    // click save button
    await E2EuiTools.clickElement('.toolbar-button.global-button-primary.eventalertpolicies-button-destination-SAVE');
  }

  async updateAlertDestination(monitoringAlertDestination: IMonitoringAlertDestination, elemenetCSS: string, elementVale: any) {
    await this.openEventAlertPolicyPage();
    // open destinatin tab
    await this.openDestionationTab();
    await browser.sleep(5000);  // wait for overlay node dismiss
    const tsTableRowActionUpdateButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringAlertDestination.meta.name) + ' .global-table-action-icon.mat-icon[mattooltip="Edit destination"]';
    await E2EuiTools.clickElement(tsTableRowActionUpdateButtonCSS);
    await browser.sleep(5000); // wait update panel show up

    await E2EuiTools.setInputBoxValue(elemenetCSS, elementVale);
    // click update-save button
    await E2EuiTools.clickElement('app-destinations app-newdestination .newdestination-buttoncontainer div:nth-child(1) > span');
  }

  async verifyAlertDestination(monitoringAlertDestination: IMonitoringAlertDestination, elemenetCSS: string, elementVale: any, isToopenDestionationTab: boolean = false) {
    // if update destination and verify updated destination are in one function block, there is no need to open Destination-Tab
    if (isToopenDestionationTab) {
      await this.openEventAlertPolicyPage();
      // open destinatin tab
      await this.openDestionationTab();
    }
    await browser.sleep(5000);  // wait for overlay node dismiss
    const tsTableRowActionUpdateButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringAlertDestination.meta.name) + ' .global-table-action-icon.mat-icon[mattooltip="Edit destination"]';
    await E2EuiTools.clickElement(tsTableRowActionUpdateButtonCSS);
    await browser.sleep(2000); // wait update panel show up
    const uiValue = await element(by.css(elemenetCSS)).getAttribute('value');
    expect(uiValue === elementVale).toBeTruthy('Alert Destination transport value should be updated properly');

  }

  async deleteAlertDestination(monitoringAlertDestination: IMonitoringAlertDestination) {
    await this.openEventAlertPolicyPage();
    // open destinatin tab
    await this.openDestionationTab();
    await browser.sleep(5000);  // wait for overlay node dismiss
    const tsTableRowActionDeleteButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringAlertDestination.meta.name) + ' .global-table-action-icon.mat-icon[mattooltip="Delete destination"]';
    await E2EuiTools.clickElement(tsTableRowActionDeleteButtonCSS);
    await browser.sleep(5000); // wait for alert pop-up
    await E2EuiTools.clickConfirmAlertFirstButton();

  }

  ////////////////////////////////////////////////

  async createEventAlertPolicy(monitoringAlertPolicy: IMonitoringAlertPolicy) {
    const appPage = new AppPage();
    appPage.setContainerCSS('.neweventalertpolicy-content');
    await this.openEventAlertPolicyPage();

    // click ' ADD ALERT POLICY' button
    await E2EuiTools.clickElement('.toolbar-button.global-button-primary.eventalertpolicies-button');
    await browser.sleep(2000);

    // set name
    await E2EuiTools.setInputBoxValue('app-eventalertpolicies .neweventalertpolicy-name.ui-inputtext', monitoringAlertPolicy.meta.name);
    const fieldSelectorCriterias: FieldSelectorCriteria[] = [];
    monitoringAlertPolicy.spec.requirements.forEach((req) => {
      fieldSelectorCriterias.push({
        key: req.key,
        operator: req.operator,
        value: req.values.join(',')
      });
    });
    await appPage.setFieldSelectorValues(fieldSelectorCriterias, 0, true);
    // click save button
    await E2EuiTools.clickElement('.toolbar-button.global-button-primary.eventalertpolicies-button-eventalertpolicy-SAVE');
  }

  async updateventAlertPolicy(monitoringAlertPolicy: IMonitoringAlertPolicy) {
    await this.openEventAlertPolicyPage();
    const tsTableRowActionUpdateButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringAlertPolicy.meta.name) + ' .global-table-action-icon.mat-icon[mattooltip="Edit policy"]';
    await E2EuiTools.clickElement(tsTableRowActionUpdateButtonCSS);
    await browser.sleep(5000); // wait update panel show up

    const dropdownCSS = 'app-eventalertpolicies  app-neweventalertpolicy .neweventalertpolicy-container p-dropdown[formControlName="severity"] .ui-dropdown-trigger > span';
    await E2EuiTools.setDropdownValue(monitoringAlertPolicy.spec.severity, 0, dropdownCSS);
    // click update-save button
    await E2EuiTools.clickElement('app-eventalertpolicies app-neweventalertpolicy .neweventalertpolicy-buttoncontainer div:nth-child(1) > span');
  }

  async verifyEventAlertPolicy(monitoringAlertPolicy: IMonitoringAlertPolicy, severityValueCSS: string, elementValue: string, isToOpenEventPolicyPage: boolean = false) {
    if (isToOpenEventPolicyPage) {
      await this.openEventAlertPolicyPage();
    }
    const tsTableRowActionUpdateButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringAlertPolicy.meta.name) + ' .global-table-action-icon.mat-icon[mattooltip="Edit policy"]';
    await E2EuiTools.clickElement(tsTableRowActionUpdateButtonCSS);
    await browser.sleep(5000); // wait update panel show up

    const uiValue = await element(by.css(severityValueCSS)).getText();
    expect(uiValue === elementValue).toBeTruthy('EventAlert Policy severity value should be updated properly');
  }

  async deleteEventAlertPolicy(monitoringAlertPolicy: IMonitoringAlertPolicy) {
    await this.openEventAlertPolicyPage();
    const tsTableRowActionDeleteButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(monitoringAlertPolicy.meta.name) + ' .global-table-action-icon.mat-icon[mattooltip="Delete policy"]';
    await E2EuiTools.clickElement(tsTableRowActionDeleteButtonCSS);
    await browser.sleep(5000); // wait for alert pop-up
    await E2EuiTools.clickConfirmAlertFirstButton();
  }


  private async openEventAlertPolicyPage() {
    // click 'Alert Policy' button
    await E2EuiTools.clickElement('.toolbar-button.global-button-primary.alertsevents-button');
    await browser.sleep(2000);
  }

  private async openDestionationTab() {
    // open destinatin tab
    await E2EuiTools.clickElement('app-alertpolicies div .mat-tab-label:nth-child(2)');
    await browser.sleep(2000);
  }
}
