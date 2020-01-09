import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { AppPage } from './page-objects/app.po';

describe('routing and guards', () => {
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


  it('Auth guard - should reroute to previous page after login', async () => {
    const until = protractor.ExpectedConditions;

    await browser.get('/#/workload');
    await loginPage.verifyPage();

    // After login we should be on workload page
    await loginPage.login();
    // Since we have services polling, angular never "stabalizes",
    // and protractor keeps waiting, so we have to manually disable waiting for angular
    await browser.waitForAngularEnabled(false);
    const shell = browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'app shell taking too long to appear in the DOM');
    const url = browser.wait(until.urlContains('/workload'), 10000, 'url did not contain /workload');
    await Promise.all([shell, url]);

    // alertpolicies page is currently disabled so the following is commented out
    // Check nested pages -
    browser.get('/#/monitoring/alertsevents/alertpolicies');
    browser.wait(until.urlContains('/monitoring/alertsevents/alertpolicies'), 10000);

    browser.refresh();
    loginPage.verifyPage();

    // // After login we should be on alertpolicies page
    loginPage.login();
    browser.waitForAngularEnabled(false);
    browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    browser.wait(until.urlContains('/monitoring/alertsevents/alertpolicies'), 10000);
  });

  it('should not load any pages protected by route guard', async () => {
    const until = protractor.ExpectedConditions;
    await browser.waitForAngularEnabled(false);
    await browser.get('/#/workload');
    await loginPage.login();
    await browser.wait(until.urlContains('/workload'), 10000, 'url did not contain /workload');
    // Should be redirected to dashboard
    await browser.get('/#/monitoring/alertsevents/alertpolicies');
    // await browser.wait(until.urlContains('/cluster/cluster'), 10000, 'did not reroute to /cluster/cluster as expected');
    expect(await element(by.css('alertpolicies')).isPresent()).toBeFalsy('app-alertpolicies was present when expected to be false');

    // check reroute from login screen
    await appPage.logout();
    await browser.get('/#/monitoring/alertsevents/alertpolicies');
    await loginPage.login();
    await browser.wait(until.urlContains('/alertsevents/alertpolicies'), 10000, 'did not reroute to /alertsevents/alertpolicies as expected');

  });

});
