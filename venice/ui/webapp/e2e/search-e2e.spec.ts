import { browser, by, element, protractor } from 'protractor';
import { SearchPage } from './page-objects/search.po';
import { LoginPage } from './page-objects/login.po';
import { AppPage } from './page-objects/app.po';


describe('venice-ui Search', () => {
    let searchPage: SearchPage;
    let loginPage: LoginPage;
    let appPage: AppPage;

    beforeEach(async (done) => {
        appPage = new AppPage();
        searchPage = new SearchPage();
        loginPage = new LoginPage();
        await loginPage.navigateTo();
        await loginPage.login();
        browser.waitForAngularEnabled(false);
        const until = protractor.ExpectedConditions;
        await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');

        done();
      });

    afterEach(async (done) => {
        // For debug only, set the browser to wait  for 10 seconds.
        const until = protractor.ExpectedConditions;
        // the url pattern does not exist. Force it to wait xxx seconds.
        //  browser.wait(until.urlContains('#'), 10000);
        try {
            await appPage.reset();
        } catch (error) {
            fail('Failed during search-e2e cleanup: ' + error);
        }
        done();
    });

    /**
    * Enter search input string and invoke search
    */
    it('should run search - user input search string -1 ', async () => {
        searchPage.setSearchInputString('is:Node');  // enter input search string
        searchPage.sendKeyStroke(protractor.Key.ENTER);
        const until = protractor.ExpectedConditions;
        // await loginPage.verifyInVenice();
        await browser.wait(until.presenceOf(element(by.css('.searchresult'))), 10000, 'test-1 .searchresult take too long to appear in DOM');
         expect(browser.getCurrentUrl()).toMatch('#/searchresult');
        // Element exists.
        expect(element(by.css('.searchresult')).isPresent()).toBeTruthy();
    });

    it('should display guided-search overlay panel -2', async () => {
        await searchPage.setSearchInputString('is:Cluster');
        await searchPage.openGuideSearchPanel();
        await browser.wait(element(by.css('.global-button-primary.guidesearch-search')).isPresent(), 10000);
        const guidedSearchButton = await  element(by.css('.global-button-primary.guidesearch-search'));
        expect(guidedSearchButton.getText()).toContain('SEARCH');
        let clusterChkbox = null;
        const catList = element.all(by.css('.guidesearch-checkbox.guidesearch-category-checkbox')).then(function (items) {
            for (let i = 0; i < items.length; i++) {
                items[i].getText().then( async (value) => {
                    if (value === 'Cluster') {
                        clusterChkbox = items[i];
                        clusterChkbox.click();
                        guidedSearchButton.click();
                        const until = protractor.ExpectedConditions;   // wait until suggestion overlay panel open
                        await browser.wait(until.presenceOf(element(by.css('.searchresult'))), 10000, 'test-2 ' + i + ' .searchresult take too long to appear in DOM');
                        expect(browser.getCurrentUrl()).toMatch('#/searchresult');
                        // Element exists.
                        expect(element(by.css('.searchresult')).isPresent()).toBeTruthy();
                    }
                });
            }
        });

    });
});
