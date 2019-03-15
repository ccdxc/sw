import { browser, by, element, protractor, WebElement, ElementFinder } from 'protractor';

export interface FieldSelectorCriteria {
    key: string;
    operator: string;
    value: string;
}

export interface TechsupportRequestValue {
    name: string;
    verbosity: number;
    nodes?: string;
    nodeSelectorValues?: FieldSelectorCriteria[];
    collectionSelectorValues?: FieldSelectorCriteria[];
}

