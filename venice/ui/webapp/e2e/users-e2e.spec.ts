import { browser, by, element, protractor, ElementFinder  } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { UsersPage } from './page-objects/users.po';
import { AppPage } from './page-objects/app.po';

import { IAuthRoleBinding, IAuthUser, IAuthRole , AuthRoleBinding, AuthUser, AuthRole  } from '@sdk/v1/models/generated/auth';
import { E2EuiTools } from './page-objects/E2EuiTools';

describe('venice-ui RBAC management', () => {
  let usersPage: UsersPage;
  let appPage: AppPage;
  let loginPage: LoginPage;

  const genAuthUser = () => {
    return new AuthUser ({
      'meta': {
        'name': 'user-' + E2EuiTools.s4()
      },
      'spec': {
        'fullname': 'u1 u1',
        'email': 'u1@pen.io',
        'password': 'Pensando0$',
        'type': 'Local'
      },
    });
  };

  const genAuthRole = () => {
      return new AuthRole(
        {
          'meta': {
            'name': 'role-' + E2EuiTools.s4()
          },
          'spec': {
            'permissions': [
              {
                'resource-tenant': 'default',
                'resource-group': 'Monitoring',
                'resource-kind': 'All',
                'actions': [
                  'All Actions'
                ]
              }
            ]
          }
        }
      );
  };

  const authRole: AuthRole = genAuthRole();
  const authUser: AuthUser = genAuthUser();

  const newuserRoleBindings = [authRole.meta.name + '_binding'];

  let originalTimeout;
  beforeEach(async (done) => {
    originalTimeout = jasmine.DEFAULT_TIMEOUT_INTERVAL;
    jasmine.DEFAULT_TIMEOUT_INTERVAL = 120000;
    appPage = new AppPage();
    usersPage = new UsersPage();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    await usersPage.navigateTo();
    await usersPage.verifyPage();
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

  it('should verify there are users', async () => {
    await usersPage.verifyUsers();
  });

  it('should verify there are roles', async () => {
    await usersPage.verifyRoles();
  });

  it('should verify there are rolebindings', async () => {
    await usersPage.verifyRolebindings();
  });

  /**
   * Add user
   * Verify user exist
   * Delete the user
   * Verify user non-exist
   */
  it('should verify add and  delete user', async () => {
    await usersPage.addUser(authUser, ['AdminRoleBinding']);
    await browser.sleep(5000); // wait for add-user operation settle.
    await usersPage.verifyUserExist(authUser, true);
    await usersPage.deleteUser(authUser);
    await browser.sleep(5000); // wait for delete-user operation settle.
    await usersPage.verifyUserExist(authUser, false);
  });

  /**
   * create a new user
   * change newly created user's password
   * logout
   * login with user's new password
   * verify the username
   * delete the user.
   */
  it('should verify user change password', async () => {
    const myAuthUser = genAuthUser();
    await browser.sleep(2000); // wait for add-user operation settle.
    await usersPage.addUser(myAuthUser, ['AdminRoleBinding']);
    await browser.sleep(5000); // wait for add-user operation settle.
    const newPassword = 'Pa55w0rd$';
    await usersPage.changeUserPassword(myAuthUser, myAuthUser.spec.password, newPassword);
    await appPage.logout();
    await browser.sleep(5000);
    await loginPage.loginWith(myAuthUser.meta.name, newPassword);
    await browser.sleep(5000);
    await appPage.verifyLoginUsername(myAuthUser.meta.name);
    await appPage.reset();
    await loginPage.login();
    await usersPage.navigateTo();
    await usersPage.verifyPage();
    await usersPage.deleteUser(myAuthUser);
  });


  it('should verify add-then-delete role', async () => {
    await usersPage.addRole(authRole);
    await browser.sleep(5000); // wait for data load-up.
    await usersPage.deleteRole(authRole);
  });



  it('should verify add-then-delete role, rolebinding and user', async () => {
    await usersPage.addRecords(authRole, authUser, newuserRoleBindings);
    await browser.sleep(2000); // wait for data load-up.
    await usersPage.deleteRecords(authRole, authUser, newuserRoleBindings);
    await browser.sleep(2000); // wait for data load-up.
  });
});
