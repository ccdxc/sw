import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { EventsEvent_severity_uihint } from '@sdk/v1/models/generated/events';

export class Alertsevents {
  constructor() {
  }

  navigateTo() {
    return browser.get('/#/monitoring/alertsevents');
  }

  async verifyPage() {
    const EC = protractor.ExpectedConditions;
    const alertseventsPageComponent = element(by.css('app-alertsevents'));
    const urlEC = EC.urlContains('/monitoring/alertsevents');
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
          expect(await colVal.getText()).not.toBe('');
        }
      }
    }
  }

}
