import { browser, by, element, protractor, WebElement } from 'protractor';
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

}
