import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { TechSupport } from './page-objects/techsupport.po';
import { AppPage } from './page-objects/app.po';
import { TechsupportRequestValue } from './page-objects/.';
import { E2EuiTools } from './page-objects/E2EuiTools';



describe('venice-ui techsupport', () => {
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
    await techsupportPage.navigateTo();
    await techsupportPage.verifyPage();
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

  const gentTechSupportRequest = () => {
    const _techsupportRequestValue: TechsupportRequestValue = {
      name: 'techsupport-' + E2EuiTools.s4(),
      verbosity: E2EuiTools.getRandomInt(0, 9),
      nodes: 'node1',
      nodeSelectorValues: [
        {
          key: 'env',
          operator: 'equals',
          value: 'test, dev'
        },
        {
          key: 'client',
          operator: 'notEquals',
          value: 'GS, JP'
        }
      ],
      collectionSelectorValues: [
        {
          key: 'version',
          operator: 'gte',
          value: '1.2'
        }
      ]
    };
    return _techsupportRequestValue;
  };


  const techsupportRequestValue = gentTechSupportRequest();

  it('should add tech-support-request record', async () => {
    await techsupportPage.createNewTechsupport(techsupportRequestValue);
    await browser.sleep(2000);
    await E2EuiTools.verifyRecordAddRemoveInTable(techsupportRequestValue.name, true);
    await browser.sleep(2000);
  });

  it('should delete tech-support-request record', async () => {
    await browser.sleep(2000); // wait for data load up.
    await techsupportPage.deleteTechsupport(techsupportRequestValue);
    await browser.sleep(2000);
      await E2EuiTools.verifyRecordAddRemoveInTable(techsupportRequestValue.name, false);
      await browser.sleep(2000);
  });

  it('should have tech-support-request records in the table', async () => {
    appPage.getTableRowLength().then(rowLen => {
      if (rowLen > 0) {
        appPage.verifyTableHasContents((rowIdx: number, columnIdx: number, rowValues: any[]) => {
          // columnIdx is 'verbosity', columnIdx is "action"
          // only column 0-name, 1-time, 4-status should be non-empty
          if (columnIdx === 0 || columnIdx === 1 || columnIdx === 4) {
            const celValue = rowValues[columnIdx];
            expect(celValue).not.toBe('', 'Tech support requests table has correct data');
          }
        });
      } else {
        expect(rowLen).toBe(0, 'There is no record found.');
      }
    });
  });



});
