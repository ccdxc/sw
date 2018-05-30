import { AppPage } from './app.po';
import { browser, by, element, protractor } from 'protractor';

describe('venice-ui App', () => {
  let page: AppPage;

  beforeEach(() => {
    page = new AppPage();
  });

  it('should display login page', () => {
    page.navigateTo();
    // Should route to login
    const EC = protractor.ExpectedConditions;
    browser.wait(EC.urlContains('/login'), 5000);
    // Expect login component
    let loginComponent = element(by.css('app-login'))
    expect(loginComponent.isPresent()).toBeTruthy();

    // Attempt to login with INCORRECT credentials
    let loginInput = element.all(by.css('.login-input'))
    loginInput.get(0).sendKeys('Liz');
    loginInput.get(1).sendKeys('wrongpassword');

    let loginButton = element(by.css('.login-signin-btn'));
    loginButton.click();

    loginComponent = element(by.css('app-login'))
    expect(loginComponent.isPresent()).toBeTruthy();

    loginInput = element.all(by.css('.login-input'))
    // Login with correct credentials
    loginInput.get(1).clear();
    loginInput.get(1).sendKeys('password');
    loginButton = element(by.css('.login-signin-btn'));
    loginButton.click();

    // Since we have services polling, angular never "stabalizes", 
    // and protractor keeps waiting, so we have to manually disable waiting for angular
    browser.waitForAngularEnabled(false)
    var until = protractor.ExpectedConditions;
    browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 5000, 'Element taking too long to appear in the DOM');
    browser.wait(EC.urlContains('/dashboard'), 5000);

    // Should not be able to read the token 
    const cookies = browser.executeScript('return document.cookie');
    expect(cookies).toEqual('');
  });
});
