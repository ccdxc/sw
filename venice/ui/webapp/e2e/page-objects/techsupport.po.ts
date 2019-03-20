import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { E2EuiTools } from './E2EuiTools';
import { AppPage } from './app.po';
import { TechsupportRequestValue} from '.';

export class TechSupport {
    constructor() {
    }

    navigateTo() {
        return browser.get('/#/monitoring/techsupport');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const techsupportPageComponent = element(by.css('app-techsupport'));
        const urlEC = EC.urlContains('/monitoring/techsupport');
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
        await browser.sleep(5000); // wati for animation complete
        await this.getTechsupportName().sendKeys(techsupportRequestValue.name);
        await this.getTechsupportVerbosity().sendKeys(techsupportRequestValue.verbosity);
        await this.getTechsupportNodes().sendKeys(techsupportRequestValue.nodes);
        await appPage4Nodes.setLabelSelectorValues(techsupportRequestValue.nodeSelectorValues);
        await appPage4Collections.setLabelSelectorValues(techsupportRequestValue.collectionSelectorValues);
        // click save button
        await this.pressSaveTechSupportButton();
    }



    async deleteTechsupport(techsupportRequestValue: TechsupportRequestValue) {
        const tsTableRowActionBlockDeleteButtonCSS = E2EuiTools.getTableEditViewTableRowActionTDCSS(techsupportRequestValue.name);
        await E2EuiTools.clickElement(tsTableRowActionBlockDeleteButtonCSS);
        await browser.sleep(2000); // wait for alert pop-up
        await E2EuiTools.clickConfirmAlertFirstButton();
    }
}
