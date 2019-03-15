import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { Naples } from './page-objects/naples.po';
import { AppPage } from './page-objects/app.po';
import { E2EuiTools } from './page-objects/E2EuiTools';

describe('venice-ui naples', () => {
    let naplesPage: Naples;
    let appPage: AppPage;
    let loginPage: LoginPage;

    beforeEach(async (done) => {
        appPage = new AppPage();
        naplesPage = new Naples();
        loginPage = new LoginPage();
        await loginPage.navigateTo();
        await loginPage.login();
        browser.waitForAngularEnabled(false);
        const until = protractor.ExpectedConditions;
        await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
        await naplesPage.navigateTo();
        await naplesPage.verifyPage();
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

    it('should have naple card records in the table', async () => {
        const rowLen = await appPage.getTableRowLength();
        await E2EuiTools.verifyUIpageTable('Naples', 'Naple');
    });

    it('should show naples statictics ', async () => {
        await browser.sleep(3000);
        const stats = await element.all(by.css('app-naples app-herocard app-basecard .herocard-first-stat-value'));
        const types = ['CPU', 'Memory', 'Storage'];
        await E2EuiTools.verifyStatistics(stats, types, 'Naples');
    });

    it('should show naple-detail', async () => {
        // $('.ui-table-scrollable-body-table tbody tr:nth-of-type(' + 1 + ') td a').text()
        const napleLink = await element(by.css('.ui-table-scrollable-body-table tbody tr:nth-of-type(1) td a')); // get the first naple
        const napleName = await napleLink.getText();
        await napleLink.click(); // go to naples/xxxx page
        await browser.sleep(3000);  // wait for browser loads data

        const stats = await element.all(by.css('app-naplesdetail app-herocard app-basecard .herocard-first-stat-value'));
        const types = ['CPU', 'Memory', 'Storage'];
        await E2EuiTools.verifyStatistics(stats, types, 'Naple Datail:' + napleName);

        // test naple-detail alert and event table
        await E2EuiTools.verifyUIpageTable('Naple Detail', 'Alert');

        // open event tab
        await E2EuiTools.clickElement('app-shared-alertsevents div .mat-tab-label:nth-child(2)');
        await E2EuiTools.verifyUIpageTable('Naple Detail', 'Event');
    });

});
