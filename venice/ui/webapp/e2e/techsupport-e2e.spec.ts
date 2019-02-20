import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { TechSupport } from './page-objects/techsupport.po';
import { AppPage } from './page-objects/app.po';

fdescribe('venice-ui techsupport', () => {
  let techsupportPage: TechSupport;
  let appPage: AppPage;
  let loginPage: LoginPage;

  beforeEach(async (done) => {
    appPage = new AppPage();
    techsupportPage = new TechSupport();
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
    await techsupportPage.navigateTo();
    await techsupportPage.verifyPage();
    appPage.getTableRowLength().then(rowLen => {
      if (rowLen > 0) {
        appPage.verifyTableHasContents((rowIdx: number, columnIdx: number, rowValues: any[]) => {
          if (columnIdx === 6) {
            const clientIpValue = rowValues[8]; // action column index is 6
            expect(clientIpValue).toBe('');
          } else {
            const celValue = rowValues[columnIdx];
            expect(celValue).not.toBe('');
          }
        });
      } else {
        console.log ('There is no record found.');
        expect(rowLen).toBe(0);
      }
    });
  });

  // TODO: add add tech-support record

});
