import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { E2EuiTools } from './E2EuiTools';
import { AppPage } from './app.po';
import { TechsupportRequestValue} from '.';

export class TechSupport {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/admin/techsupport');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const techsupportPageComponent = element(by.css('app-techsupport'));
        const urlEC = EC.urlContains('/admin/techsupport');
        const componentEC = EC.presenceOf(techsupportPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }

    getTechsupportName() {
        return element(by.css('.newtechsupport-name.ui-inputtext'));
    }

    getTechsupportVerbosity() {
        return element(by.css('.newtechsupport-verbosity.ui-inputtext'));
    }

    getTechsupportNodes() {
        return element(by.css('.newtechsupport-nodes.ui-inputtext'));
    }


    async pressAddTechSupportButton() {
        const saveTechsupportButton = await element(by.css('.toolbar-button.global-button-primary.techsupportrequests-toolbar-button.techsupportrequests-toolbar-button-ADD'));
        await saveTechsupportButton.click();
    }

    async pressSaveTechSupportButton() {
        const saveTechsupportButton = await element(by.css('.toolbar-button.global-button-primary.techsupportrequests-toolbar-button.techsupportrequests-toolbar-SAVE'));
        await saveTechsupportButton.click();
    }

    async createNewTechsupport(techsupportRequestValue: TechsupportRequestValue) {
        const appPage4Nodes = new AppPage();
        appPage4Nodes.setContainerCSS ('.newtechsupport-row-nodeselector');
        const appPage4Collections = new AppPage();
        appPage4Collections.setContainerCSS ('.newtechsupport-row-collectionselector');
        await this.pressAddTechSupportButton() ;
        await browser.sleep(2000); // wati for animation complete
        await this.getTechsupportName().sendKeys(techsupportRequestValue.name);
        const nodeList = 'p-listbox .ui-listbox-list-wrapper .ui-listbox-item > span';
        await E2EuiTools.setListBoxValue(techsupportRequestValue.nodes.split(','), nodeList);
        // click save button
        await this.pressSaveTechSupportButton();
    }



    async deleteTechsupport(techsupportRequestValue: TechsupportRequestValue) {
        const tsTableRowActionBlockDeleteButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(techsupportRequestValue.name);
        await E2EuiTools.clickElement(tsTableRowActionBlockDeleteButtonCSS);
        await E2EuiTools.clickConfirmAlertFirstButton();
    }
}
