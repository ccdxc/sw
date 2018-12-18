import { browser, by, element, protractor } from 'protractor';

export class SearchPage {
    searchInput = element.all(by.css('.searchwidget .ui-autocomplete-input-token .ui-autocomplete-input-text'));

    setSearchInputString(inputString: string, append?: false) {
        if (append ) {
                this.searchInput.clear();
        }
        this.searchInput.sendKeys(inputString);
    }

    // stroke is like protractor.Key.ENTER
    sendKeyStroke(stroke: any) {
        this.searchInput.sendKeys(stroke);
    }

    getRemoveAllIcon (): any {
        return element.all(by.css('.searchwidget .searchwidget-autocomplete-token-icon-rmAll'));
    }

    getSearchIcon (): any {
        return element.all(by.css('.searchwidget .searchwidget-autocomplete-token-icon-search'));
    }

    getGuidedSearchIcon (): any {
        return element.all(by.css('.searchwidget .searchwidget-autocomplete-token-icon-guided'));
    }

    openGuideSearchPanel() {
        const iconGuidedSearch = this.getGuidedSearchIcon();
        iconGuidedSearch.click();
    }
}
