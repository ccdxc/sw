import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { AppPage } from './page-objects/app.po';

describe('venice-ui Login', () => {
  let loginPage: LoginPage;
  let appPage: AppPage;

  beforeEach(async (done) => {
    loginPage = new LoginPage();
    appPage = new AppPage();
    done();
  });

  afterEach(async (done) => {
    try {
      await appPage.reset();
    } catch (error) {
      fail('Failed during cleanup: ' + error);
    }
    done();
  });

  it('should display login page and go to homepage', async () => {

    await loginPage.navigateTo();
    // Should route to login
    await loginPage.verifyPage();

    // Attempt to login with INCORRECT credentials
    await loginPage.loginWith('Liz', 'wrongpassword');
    await loginPage.verifyFailedLogin();

    // Login with correct credentials
    await loginPage.login();

    // Since we have services polling, angular never "stabalizes",
    // and protractor keeps waiting, so we have to manually disable waiting for angular
    await browser.waitForAngularEnabled(false);
    const EC = protractor.ExpectedConditions;
    await browser.wait(EC.presenceOf(element(by.css('.app-shell-container'))), 60000, 'app shell taking too long to appear in the DOM');
    await browser.wait(EC.urlContains('/cluster/cluster'), 10000, 'did not reroute to /cluster/cluster as expected');

    // Should not be able to read the token
    const cookies = browser.executeScript('return document.cookie');
    expect(await cookies).toEqual('');
  });

  it('should go to login page on refresh', async () => {
    await appPage.reset();
    // initial login
    await loginPage.navigateTo();
    await loginPage.verifyPage();
    await loginPage.login();

    // Since we have services polling, angular never "stabalizes",
    // and protractor keeps waiting, so we have to manually disable waiting for angular
    await browser.waitForAngularEnabled(false);
    const EC = protractor.ExpectedConditions;
    const shell = browser.wait(EC.presenceOf(element(by.css('.app-shell-container'))), 10000, 'app shell taking too long to appear in the DOM');
    const url = browser.wait(EC.urlContains('/cluster/cluster'), 10000, 'did not reroute to /cluster/cluster as expected');
    await Promise.all([shell, url]);

    // Refresh the page and go back to login
    await browser.refresh();
    await appPage.verifyInVenice();  // Previously we call - loginPage.verifyPage();  As we change login logic, new we just want to verify we are in Venice.

  });

});
