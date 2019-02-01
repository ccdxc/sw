import { browser, by, element, protractor } from 'protractor';

export class LoginPage {
  username: string;
  password: string;

  constructor(username: string = 'test', password: string = 'pensando') {
    this.username = username;
    this.password = password;
  }

  getLoginInput() {
    return element.all(by.css('.login-input'));
  }

  getLoginButton() {
    return element(by.css('.login-signin-btn'));
  }

  navigateTo() {
    return browser.get('/#/login');
  }

  async verifyPage() {
    const EC = protractor.ExpectedConditions;
    const loginComponent = element(by.css('app-login'));
    await browser.wait(EC.urlContains('/login'), 10000, 'login.spec.verifyPage() - url did not contain /login');
    expect(await loginComponent.isPresent()).toBeTruthy('login component was not present');
  }



  async verifyFailedLogin() {
    const EC = protractor.ExpectedConditions;
    const loginComponent = element(by.css('app-login'));
    const loginError = element(by.css('.login-error'));
    await browser.wait(EC.urlContains('/login'), 10000, 'url did not contain /login');
    expect(await loginComponent.isPresent()).toBeTruthy('login component was not present');
    await browser.wait(EC.presenceOf(loginError), 10000, 'login error message was not present');
  }

  async setUsernameInput(user: string) {
    const loginInput = this.getLoginInput();
    const EC = protractor.ExpectedConditions;
    await browser.wait(EC.presenceOf(loginInput.get(0)), 10000, 'login username input never appeared');
    await loginInput.get(0).clear();
    await loginInput.get(0).sendKeys(user);
  }

  async setPasswordInput(password: string) {
    const loginInput = this.getLoginInput();
    const EC = protractor.ExpectedConditions;
    await browser.wait(EC.presenceOf(loginInput.get(1)), 10000, 'login password input never appeared');
    await loginInput.get(1).clear();
    await loginInput.get(1).sendKeys(password);
  }

  login() {
    return this.loginWith(this.username, this.password);
  }

  async loginWith(username: string, password: string) {
    const EC = protractor.ExpectedConditions;
    await this.setUsernameInput(username);
    await this.setPasswordInput(password);
    await browser.wait(EC.elementToBeClickable(this.getLoginButton()), 10000, 'Login button never became clickable');
    await this.getLoginButton().click();
  }

}
