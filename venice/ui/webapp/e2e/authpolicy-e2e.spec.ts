import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { AuthPolicy } from './page-objects/authpolicy.po';
import { AppPage } from './page-objects/app.po';

/**
 *  This e2e-ui test AuthPolicy UI.
 * Please note:  This AuthPolicy e2e-ui test will change Venice configuration. It is intrusive and e2e test can not restore the original data. Thus, we use "xdescribe" to exclude it. Run it manually.
 *
 *  AuthPolicy is a single (per tenant) object. UI does not know what it is like ahead of time. It should aleast has LOCAL config.
 *  Thus, we are trying to add RADIUS or LDAP configs to AuthPolicy.
 *  Then we test changing config orders.
 */
xdescribe('venice-ui auth-policy', () => {
  let authPolicyPage: AuthPolicy;
  let appPage: AppPage;
  let loginPage: LoginPage;

  let originalTimeout;

  beforeEach(async (done) => {
    originalTimeout = jasmine.DEFAULT_TIMEOUT_INTERVAL;
    jasmine.DEFAULT_TIMEOUT_INTERVAL = 120000;
    appPage = new AppPage();
    authPolicyPage = new AuthPolicy();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    await authPolicyPage.navigateTo();
    await authPolicyPage.verifyPage();
    done();
  });

  afterEach(async (done) => {
    jasmine.DEFAULT_TIMEOUT_INTERVAL = originalTimeout;
    try {
      await appPage.reset();
      done();
    } catch (error) {
      fail('Failed during cleanup: ' + error);
    }
  });

  /**
   * Try to add RADIUS Configuration.
   */
  it('should see RADIUS config or add RADIUS config', async () => {
    // We first find out if RADIUS is already configured by locating RADIUS-AUTHENTICATION-BUTTON.
    const radiusCreateButton = await authPolicyPage.getCreateRADIUSAuthenticationeButton();
    const radiusConfigData = await authPolicyPage.getRadiusConfigData();  // using await is the key
    if (radiusCreateButton === null) {
      console.log('Auth Policy includes RADIUS configurations already.');
      expect(radiusCreateButton).not.toBeNull();
    } else {
      // radiusCreateButton is not reliable. we have to check if radiusConfigData is in page.
      if (radiusConfigData.length === 0) {
        console.log('Auth Policy does not include RADIUS configuration. Start adding RADIUS Config');
        await authPolicyPage.addRADIUSConfig();
      } else {
        console.log('Auth Policy includes RADIUS configurations already. There is RADIUS config data');
        expect(radiusConfigData.length).toBeGreaterThan(0);
      }
    }
  });

  /** See if AuthPolicy includes LDAP Config.  If not, add it */
  it('should see LDAP config or add LDAP config', async () => {
    // We first find out if ldap is already configured by locating ldap-AUTHENTICATION-BUTTON.
    const ldapCreateButton = await authPolicyPage.getCreateLDAPAuthenticationeButton();
    const ldapConfigData = await authPolicyPage.getLDAPConfigData();  // using await is the key
    if (ldapCreateButton === null) {
      console.log('Auth Policy includes LDAP configurations already.');
      expect(ldapCreateButton).not.toBeNull();
    } else {
      // radiusCreateButton is not reliable. we have to check if radiusConfigData is in page.
      if (ldapConfigData.length === 0) {
        console.log('Auth Policy does not include LDAP configuration. Start adding LDAP Config');
        await authPolicyPage.addLDAPConfig();
      } else {
        console.log('Auth Policy includes LDAP configurations already. There is LDAP config data');
        expect(ldapConfigData.length).toBeGreaterThan(0);
      }
    }
  });

  /**
   * Test LDAP BIND and SERVER connections
   */
  it('should test LDAP BIND Connection', async () => {
    await authPolicyPage.testLDAPBindConnection();
  });

  /**
   * Try to change configuratino orders
   */
  it('should change configs order', async () => {
    const prevConfigTitles = await authPolicyPage.getConfigTitle();
    this.oldConfigTitles = prevConfigTitles;
    if (prevConfigTitles.length > 1) {
      await authPolicyPage.move2ndConfigUp();
    } else {
      console.log('AuthPolicy only has ' + prevConfigTitles.length + ' config. We need aleast two configs to change orders');
      expect(prevConfigTitles.length).toBeLessThan(2);
    }
  });

  /**
   * Verify config order change
   */
  it('should verify new config orders', async () => {
    const newConfigTitles = await authPolicyPage.getConfigTitle();
    if (newConfigTitles.length > 1) {
      expect(this.oldConfigTitles[0] === newConfigTitles[1]);
    } else {
      console.log('AuthPolicy only has ' + newConfigTitles.length + ' config. We need aleast two configs to verify orders');
      expect(newConfigTitles.length).toBeLessThan(2);
    }
  });
});

