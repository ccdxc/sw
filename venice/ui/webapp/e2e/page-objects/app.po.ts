import { browser, by, element, protractor, WebElement, ElementFinder } from 'protractor';
import { LoginPage } from './login.po';
import { By } from 'selenium-webdriver';
import { FieldSelectorCriteria, TechsupportRequestValue } from '.';
import { E2EuiTools } from './E2EuiTools';

export class AppPage {

  containerCSS: string = '';

  constructor() {
  }

  setContainerCSS(containerCSS: string) {
    this.containerCSS = containerCSS;
  }

  async logout() {
    await this._openLoginNamePanel();
    // sleeping to make sure it registers hover state before clicking
    await browser.sleep(1000);
    await browser.actions().click().perform();
    const loginPage = new LoginPage();
    await loginPage.verifyPage();
  }

  public async _openLoginNamePanel() {
    const EC = protractor.ExpectedConditions;
    const userButton = element(by.css('.app-user-button'));
    await browser.wait(EC.presenceOf(userButton), 10000, 'User button was not present');
    await userButton.click();
    const logoutButton = element(by.css('.app-logout-button'));
    await browser.wait(EC.presenceOf(logoutButton), 10000, 'Logout button was not present');
    // sleeping to make sure animation finishes
    await browser.sleep(2000);
    const button = browser.driver.findElement(By.css('.app-logout-button'));
    await browser.actions().mouseMove(await button).perform();
  }

  async verifyLoginUsername(username: string) {
    await this._openLoginNamePanel();
    const loginLabel = await element(by.css('.app-shell-toolbar-user-menu-name')).getText();
    const loginName = loginLabel.split(':')[1].trim();  // LoginLabel is like "User:test"
    expect(loginName === loginName).toBeTruthy();
    await E2EuiTools.clickElement('.app-shell-toolbar-user-menu-name');
  }

  async verifyLoggedIn() {
    const EC = protractor.ExpectedConditions;
    await browser.wait(EC.presenceOf(element(by.css('.app-shell-container'))), 10000, 'app shell taking too long to appear in the DOM');
  }

  async reset() {
    // Resetting navigation and forcing a logout
    const url = await browser.getCurrentUrl();
    if (url.includes('login')) {
      await browser.refresh();
    } else {
      await this.logout();
    }
  }

  async verifyInVenice() {
    const EC = protractor.ExpectedConditions;
    const appShellComponent = element(by.css('.app-shell-container'));
    expect(await appShellComponent.isPresent()).toBeTruthy(); // VeniceUI should be in logged-in stage.
  }

  /**
   * This is a common API to verify table has content. Every table cell is not empty
   */
  async verifyTableHasContents(onCell: (rowIdx: number, columnIdx: number, rowValues: any[]) => any = null,
    onComplete: () => any = null
  ) {
    const tableData = await this.getTableContent();
    const limit = tableData.length;
    const list = [];
    for (let rowIndex = 0; rowIndex < limit; rowIndex++) {

      const recValues = tableData[rowIndex];
      for (let colIndex = 0; colIndex < recValues.length; colIndex++) {
        if (onCell != null) {
          onCell(rowIndex, colIndex, recValues);
        }
      }
    }
    if (onComplete) {
      await browser.sleep(2000);
      onComplete();
    }
    return list; // all data in table. It is a maxtrix.
  }

  async getTableContent( hasLimit: boolean = false ) {
    const EC = protractor.ExpectedConditions;
    await browser.wait(element(by.css('.ui-table-scrollable-body-table tbody tr td')).isPresent(), 5000);
    // Let rendering finish
    await browser.sleep(1000);
    const rows = await element.all(by.css('.ui-table-scrollable-body-table tbody tr'));
    let limit = rows.length;
    // Limiting to first 10 events due to the maount of time it takes
    // to check each row
    if (hasLimit && limit > 100) {
      limit = 100;
    }
    const list = [];
    for (let rowIndex = 0; rowIndex < limit; rowIndex++) {
      // We re select the element to avoid our reference being stale
      // nth-of-type is 1 based.
      const colVals = await element.all(by.css('.ui-table-scrollable-body-table tbody tr:nth-of-type(' + (rowIndex + 1) + ') td'));
      const recValues = [];
      list.push(recValues);
      // collect all columns value
      for (let colIndex = 0; colIndex < colVals.length; colIndex++) {
        const colVal: ElementFinder = colVals[colIndex];
        const colText = await colVal.getText();
        recValues.push(colText);
      }
    }
    return list; // all data in table. It is a maxtrix.
  }


  async getTableRowLength() {
    const EC = protractor.ExpectedConditions;
    await browser.wait(element(by.css('.ui-table-scrollable-body-table tbody tr td')).isPresent(), 5000);
    // Let rendering finish
    await browser.sleep(1000);
    const rows = await element.all(by.css('.ui-table-scrollable-body-table tbody tr'));
    return rows.length;
  }

  async _openFieldSelectorKeyDropDownPanel(rowIndex: number = 0) {
    const fieldSelectorArrows = await element.all(by.css(this.containerCSS + ' ' + '.repeater-selector.repeater-key .ui-dropdown-trigger.ui-corner-right .ui-dropdown-trigger-icon.ui-clickable'));
    await fieldSelectorArrows[rowIndex].click();
  }

  async _openFieldSelectorOperatorDropDownPanel(rowIndex: number = 0) {
    const operatorSelectorArrows = await element.all(by.css(this.containerCSS + ' ' + '.repeater-selector.repeater-operator .ui-dropdown-trigger.ui-corner-right .ui-dropdown-trigger-icon.ui-clickable'));
    await operatorSelectorArrows[rowIndex].click();
  }

  async _setFieldSelectorValueInput(value: string, rowIndex: number = 0) {
    const valueInputs = await element.all(by.css(this.containerCSS + ' ' + '.repeater-selector.repeater-value .repeater-input.ui-inputtext'));
    await valueInputs[rowIndex].sendKeys(value);
  }

  async _setFieldSelectorOperator(operator: string, index: number, myopCSS: string = null) {
    const opCSS = (myopCSS) ? myopCSS : '.ng-trigger.ng-trigger-overlayAnimation.ui-dropdown-panel.ui-widget .ui-dropdown-items-wrapper .ui-dropdown-item > span';
    await this._openFieldSelectorOperatorDropDownPanel(index);
    const opsElements = await element.all(by.css(opCSS));
    const opsTexts = await element.all(by.css(opCSS)).getText();
    const opOption = E2EuiTools.findElementFinder(opsElements, opsTexts, operator);
    await opOption.click();
  }

  async _setFieldSelectorKey(key: string, index: number, myKeyCSS: string = null) {
    const keyCSS = (myKeyCSS) ? myKeyCSS : '.ng-trigger.ng-trigger-overlayAnimation.ui-dropdown-panel.ui-widget .ui-dropdown-items-wrapper .ui-dropdown-item';
    await this._openFieldSelectorKeyDropDownPanel(index);
    const keysElements = await element.all(by.css(keyCSS));
    const keyTexts = await element.all(by.css(keyCSS)).getText();
    const actionOption = E2EuiTools.findElementFinder(keysElements, keyTexts, key);
    await actionOption.click();
  }

  /** Label Selector  */
  async _openLabelSelectorOperatorDropDownPanel(rowIndex: number = 0) {
    const operatorSelectorArrows = await element.all(by.css(this.containerCSS + ' ' + '.repeater-selector.repeater-operator .ui-dropdown-trigger.ui-corner-right .ui-dropdown-trigger-icon.ui-clickable'));
    await operatorSelectorArrows[rowIndex].click();
  }

  async _setLabelSelectorOperator(operator: string, index: number, myopCSS: string = null) {
    const opCSS = (myopCSS) ? myopCSS : '.ng-trigger.ng-trigger-overlayAnimation.ui-dropdown-panel.ui-widget .ui-dropdown-items-wrapper .ui-dropdown-item > span';
    await this._openFieldSelectorOperatorDropDownPanel(index);
    const opsElements = await element.all(by.css(opCSS));
    const opsTexts = await element.all(by.css(opCSS)).getText();
    const opOption = E2EuiTools.findElementFinder(opsElements, opsTexts, operator);
    await opOption.click();
  }

  async _setLabelSelectorKey(key: string, rowIndex: number, myKeyCSS: string = null) {
    const keyCSS = (myKeyCSS) ? myKeyCSS : this.containerCSS + ' ' + '.repeater-input.ui-inputtext[ng-reflect-name="keytextFormName"]';
    const labelKeyElements = await element.all(by.css(keyCSS));
    await labelKeyElements[rowIndex].sendKeys(key);
  }

  async _setLabelSelectorValueInput(value: string, rowIndex: number, myVaueCSS: string = null) {
    const valueCSS = (myVaueCSS) ? myVaueCSS : this.containerCSS + ' ' + '.repeater-input.ui-inputtext[ng-reflect-name="valueFormControl"]';
    const labelValueElements = await element.all(by.css(valueCSS));
    await labelValueElements[rowIndex].sendKeys(value);
  }

  /**
   *
   * @param fieldSelectorCriterias
   * @param startRowIndex
   */
  async setFieldSelectorValues(fieldSelectorCriterias: FieldSelectorCriteria[], startRowIndex = 0) {
    await this.addRepeaterRows(fieldSelectorCriterias, startRowIndex);

    for (let i = 0; i < fieldSelectorCriterias.length; i++) {
      const fieldSelectorCriteria = fieldSelectorCriterias[i];

      // set Field selector KEY
      await this._setFieldSelectorKey(fieldSelectorCriteria.key, startRowIndex + i);

      await browser.sleep(2000); // This wait is important as key change will invoke reload operators.

      await this._setFieldSelectorOperator(fieldSelectorCriteria.operator, startRowIndex + i);
      // set Field selector value
      await this._setFieldSelectorValueInput(fieldSelectorCriteria.value, startRowIndex + i);
      await browser.sleep(1000);
    }

  }

  /**
   * Click "ADD" sign multiple times.
   * @param fieldSelectorCriterias
   * @param startRowIndex
   *
   * Say:
   *  FieldSelector already has 2 rows and we when to add 3 more rows. we will hit the 2nd row's "Add" button 3 times.
   *  FieldSelector has 1 empty rows and we when to add 3 more rows. we will hit the 0 row's "Add" button 2 times.
   *
   */
  private async addRepeaterRows(fieldSelectorCriterias: FieldSelectorCriteria[], startRowIndex: number) {
    const addSigns = await element.all(by.css(this.containerCSS + ' ' + '.repeater-and >span'));
    let addClickTimes = fieldSelectorCriterias.length;
    if (startRowIndex === 0) {
      addClickTimes = addClickTimes - 1;
    }
    const lastAddSign = (startRowIndex === 0) ? addSigns[0] : addSigns[startRowIndex - 1];
    for (let i = 0; i < addClickTimes; i++) {
      await lastAddSign.click();
    }
  }

  async setLabelSelectorValues(fieldSelectorCriterias: FieldSelectorCriteria[], startRowIndex = 0) {
    await this.addRepeaterRows(fieldSelectorCriterias, startRowIndex);

    for (let i = 0; i < fieldSelectorCriterias.length; i++) {
      const fieldSelectorCriteria = fieldSelectorCriterias[i];

      // set Label selector KEY
      await this._setLabelSelectorKey(fieldSelectorCriteria.key, startRowIndex + i);

      await this._setLabelSelectorOperator(fieldSelectorCriteria.operator, startRowIndex + i);
      // set Label selector value
      await this._setLabelSelectorValueInput(fieldSelectorCriteria.value, startRowIndex + i);
      await browser.sleep(1000);
    }
  }

}
