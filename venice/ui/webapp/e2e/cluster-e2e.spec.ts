import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { Cluster } from './page-objects/cluster.po';
import { AppPage } from './page-objects/app.po';
import { E2EuiTools } from './page-objects/E2EuiTools';

describe('venice-ui cluster', () => {
    let clusterPage: Cluster;
    let appPage: AppPage;
    let loginPage: LoginPage;

    beforeEach(async (done) => {
        appPage = new AppPage();
        clusterPage = new Cluster();
        loginPage = new LoginPage();
        await loginPage.navigateTo();
        await loginPage.login();
        browser.waitForAngularEnabled(false);
        const until = protractor.ExpectedConditions;
        await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
        await clusterPage.navigateTo();
        await clusterPage.verifyPage();
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

    it('should have alert  and event records in the table', async () => {
        const rowLen = await appPage.getTableRowLength();
        await E2EuiTools.verifyUIpageTable('Cluster', 'Alert');

        // open event tab
        await E2EuiTools.clickElement('app-shared-alertsevents div .mat-tab-label:nth-child(2)');
        await E2EuiTools.verifyUIpageTable('Cluster', 'Event');
    });

    it('should show cluster statictics ', async () => {
        const stats = await element.all(by.css('app-cluster app-herocard app-basecard .herocard-first-stat-value'));
        const types = ['CPU', 'Memory', 'Storage'];
        await E2EuiTools.verifyStatistics(stats, types, 'Cluster');
    });

    it('should show cluster-node ', async () => {
        const nodelink = await element(by.css('app-cluster .cluster-node-container .cluster-node-name span')); // get the first node
        const nodeName = await nodelink.getText();
        await nodelink.click(); // go to cluster/nodeID page
        await browser.sleep(3000);  // wait for browser loads data

        const stats = await element.all(by.css('app-nodedetail app-herocard app-basecard .herocard-first-stat-value'));
        const types = ['CPU', 'Memory', 'Storage'];
        await E2EuiTools.verifyStatistics(stats, types, 'Node Datail:' + nodeName);

        // test naple-detail alert and event table
        await E2EuiTools.verifyUIpageTable('Node Detail', 'Alert');

        // open event tab
        await E2EuiTools.clickElement('app-shared-alertsevents div .mat-tab-label:nth-child(2)');
        await E2EuiTools.verifyUIpageTable('Node Detail', 'Event');
    });

});
