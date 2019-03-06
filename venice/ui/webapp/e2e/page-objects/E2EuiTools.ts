import { browser, by, element, protractor, WebElement, ElementFinder } from 'protractor';
import { throws } from 'assert';

export class E2EuiTools {
    public static s4() {
        return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
    }

    public static getRandomInt(min: number, max: number) {
        return Math.floor(Math.random() * (max - min + 1)) + min;
    }
    public static findElementFinder(keysElements: ElementFinder[], keyTexts: any, text: string): ElementFinder {
        let actionOption: ElementFinder = null;
        for (let i = 0; i < keyTexts.length; i++) {
            const keyText = keyTexts[i];
            if (keyText === text) {
                actionOption = keysElements[i];
                break;
            }
        }
        return actionOption;
    }

    public static async openDropdownPanel(dropdownArrowCSS: string, rowIndex: number = 0) {
        const dropdownArrows = await element.all(by.css(dropdownArrowCSS));
        await dropdownArrows[rowIndex].click();
    }

    public static buildError(source: string, message: string, parameters: any[]) {
        const obj = {
            source: source,
            message: message,
            parameters: parameters
        };
        return new Error(JSON.stringify(obj, null, 2));
    }

    public static async setDropdownValue(dropdownValue: string, index: number, dropdownArrowCSS: string, dropdownOptionCSS: string = null) {
        const opCSS = (dropdownOptionCSS) ? dropdownOptionCSS : '.ng-trigger.ng-trigger-overlayAnimation.ui-dropdown-panel.ui-widget .ui-dropdown-items-wrapper .ui-dropdown-item > span';
        await this.openDropdownPanel(dropdownArrowCSS, index);
        await browser.sleep(1000); // wait until drowdown slide down animimation completed.
        const opsElements = await element.all(by.css(opCSS));
        const opsTexts = await element.all(by.css(opCSS)).getText();
        const opOption = this.findElementFinder(opsElements, opsTexts, dropdownValue);
        if (opOption) {
            await opOption.click();
        } else {
            throw this.buildError('E2EuiTools.setDropdownValue()', 'Can not find option', [
                {
                    dropdownValue: dropdownValue,
                    index: index,
                    dropdownArrowCSS: dropdownArrowCSS,
                    dropdownOptionCSS: dropdownOptionCSS
                }
            ]);
        }
    }

    public static async setListBoxValue(listboxValues: string[], listboxCSS: string) {
        const opsElements = await element.all(by.css(listboxCSS));
        const opsTexts = await element.all(by.css(listboxCSS)).getText();
        for (let i = 0; i < listboxValues.length; i++) {
            const opOption = this.findElementFinder(opsElements, opsTexts, listboxValues[i]);
            await opOption.click();
        }
    }

    public static async clickElement(cssPath: string) {
        const elementButton = await element(by.css(cssPath));
        await elementButton.click();
    }

    public static async setInputBoxValue(cssPath: string, value: string) {
        const elementInput = await element(by.css(cssPath));
        await elementInput.clear();
        await elementInput.sendKeys(value);
    }

    public static async setMultipleInputBoxValues(cssPath: string, values: string[]) {
        const elementInputs = await element.all(by.css(cssPath));
        for (let i = 0; i < values.length; i++) {
            const elementInput = elementInputs[i];
            await elementInput.clear();
            await elementInput.sendKeys(values[i]);
        }
    }

    public static async setMultiSelectDropdownValue(selectedValues: string[], index: number, dropdownArrowCSS: string, dropdownOptionCSS: string = null, useDoubleClick: boolean = false) {
        const opCSS = (dropdownOptionCSS) ? dropdownOptionCSS : '.ng-trigger.ng-trigger-overlayAnimation.ui-multiselect-panel.ui-widget.ui-widget-content .ui-multiselect-items-wrapper label';
        await this.openDropdownPanel(dropdownArrowCSS, index);
        const opsElements = await element.all(by.css(opCSS));
        const opsTexts = await element.all(by.css(opCSS)).getText();
        for (let i = 0; i < selectedValues.length; i++) {
            const opOption = this.findElementFinder(opsElements, opsTexts, selectedValues[i]);
            await opOption.click();
        }
    }

    public static async clickConfirmAlertFirstButton() {
        const buttonCSS = 'p-confirmdialog .ui-dialog-footer.ui-widget-content  > button:nth-child(1)';
        await this.clickElement(buttonCSS);
    }
}
