import { browser, by, element, protractor, WebElement, ElementFinder } from 'protractor';
import { LoginPage } from './login.po';
import { By } from 'selenium-webdriver';

export class AppPage {
  constructor() {
  }

  async logout() {
    const EC = protractor.ExpectedConditions;
    const userButton = element(by.css('.app-user-button'));
    await browser.wait(EC.presenceOf(userButton), 10000, 'User button was not present');
    await userButton.click();
    const logoutButton = element(by.css('.app-logout-button'));
    await browser.wait(EC.presenceOf(logoutButton), 10000, 'Logout button was not present');
    // sleeping to make sure animation finishes
    await browser.sleep(2000);
    const button = browser.driver.findElement(By.css('.app-logout-button'));
    await browser.actions().mouseMove(await button).perform();
    // sleeping to make sure it registers hover state before clicking
    await browser.sleep(1000);
    await browser.actions().click().perform();
    const loginPage = new LoginPage();
    await loginPage.verifyPage();
  }

  async verifyLoggedIn() {
    const EC = protractor.ExpectedConditions;
    await browser.wait(EC.presenceOf(element(by.css('.app-shell-container'))), 10000, 'app shell taking too long to appear in the DOM');
  }

  async reset() {
    // Resetting navigation and forcing a logout
    const url = await browser.getCurrentUrl();
    if (url.includes('login')) {
      await browser.refresh();
    } else {
      await this.logout();
    }
  }

  async verifyInVenice() {
    const EC = protractor.ExpectedConditions;
    const appShellComponent = element(by.css('.app-shell-container'));
    expect(await appShellComponent.isPresent()).toBeGreaterThan(0);  // VeniceUI should be in logged-in stage.
  }

  /**
   * This is a common API to verify table has content. Every table cell is not empty
   */
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
        expect(colText).not.toBe('');
      }
    }
  }

}
