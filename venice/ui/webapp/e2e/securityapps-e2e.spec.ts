import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { SecurityApps } from './page-objects/securityapps.po';
import { AppPage } from './page-objects/app.po';

describe('venice-ui security-apps', () => {
    let securityappsPage: SecurityApps;
    let appPage: AppPage;
    let loginPage: LoginPage;

    beforeEach(async (done) => {
        appPage = new AppPage();
        securityappsPage = new SecurityApps();
        loginPage = new LoginPage();
        await loginPage.navigateTo();
        await loginPage.login();
        browser.waitForAngularEnabled(false);
        const until = protractor.ExpectedConditions;
        await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
        await securityappsPage.navigateTo();
        await securityappsPage.verifyPage();
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

    it('should have security-app records in the table', async () => {
        const rowLen = await appPage.getTableRowLength();
        if (rowLen > 0) {
            const tableData = await appPage.getTableContent();
            expect(tableData.length > 0).toBeTruthy('cluster table should have records');
        } else {
            expect(rowLen).toBe(0, 'cluster table  has no record');
        }
    });

});
