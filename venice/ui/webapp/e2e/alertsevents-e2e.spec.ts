import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { MonitoringAlertPolicy, MonitoringAlertPolicySpec_severity } from '@sdk/v1/models/generated/monitoring';
import { Alertsevents } from './page-objects/alertsevents.po';
import { AppPage } from './page-objects/app.po';

describe('venice-ui alertsevents', () => {
  let alertseventsPage: Alertsevents;
  let appPage: AppPage;
  let loginPage: LoginPage;

  beforeEach(async (done) => {
    appPage = new AppPage();
    alertseventsPage = new Alertsevents();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    done();
  });

  afterEach(async (done) => {
    try {
      await appPage.reset();
      done();
    } catch (error) {
      fail('Failed during cleanup: ' + error);
    }
  });

  it('should have events in the table', async () => {
    await alertseventsPage.navigateTo();
    await alertseventsPage.verifyPage();
    await alertseventsPage.verifyTableHasContents();
  });

});
