import { Component, EventEmitter, OnInit, Output, ViewChild, ViewEncapsulation, Input, OnChanges } from '@angular/core';
import { FormArray, FormControl } from '@angular/forms';
import { Animations } from '@app/animations';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { Utility } from '@app/common/Utility';
import { RepeaterComponent, RepeaterData, ValueType, RepeaterItem } from 'web-app-framework';
import { SearchSpec, SearchExpression, SearchModelField, GuidedSearchCriteria } from '@app/components/search';

@Component({
  selector: 'app-guidedsearch',
  templateUrl: './guidedsearch.component.html',
  styleUrls: ['./guidedsearch.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class GuidesearchComponent implements OnInit, OnChanges {

  @ViewChild('fieldRepeater') fieldRepeater: RepeaterComponent;
  @ViewChild('labelRepeater') labelRepeater: RepeaterComponent;

  selectedCategories: any[] = [];
  categorySuggestions: any[] = [];

  selectedKinds: any[] = [];
  kindSuggestions: any[] = [];

  fieldData: RepeaterData[] = [];
  labelData: RepeaterData[] = [];

  fieldOutput: any;
  labelOutput: any;

  fieldFormArray = new FormArray([]);
  labelFormArray = new FormArray([]);

  isToSelectField: boolean = false;
  isToSelectLabel: boolean = false;


  @Input() inputConfig: SearchSpec;
  @Output() proceedGuidedSearch: EventEmitter<GuidedSearchCriteria> = new EventEmitter();

  constructor() { }

  getClassName(): string {
    return this.constructor.name;
  }

  ngOnChanges() {
    if (this.inputConfig) {
      this.populate(this.inputConfig);
    }
  }

  ngOnInit() {
    this.categorySuggestions = SearchUtil.getCategories();
    this.kindSuggestions = SearchUtil.getKinds();
    this.labelData = [
      {
        key: { label: 'text', value: 'text' },
        operators: SearchUtil.stringOperators,
        fieldType: ValueType.inputField,
        valueType: ValueType.inputField
      }
    ];

    this.fieldData = [
      {
        key: { label: 'name', value: 'name' },
        operators: SearchUtil.stringOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'tenant', value: 'tenant' },
        operators: SearchUtil.stringOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'namespace', value: 'namespace' },
        operators: SearchUtil.stringOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'creation-time', value: 'creation-time' },
        operators: SearchUtil.numberOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'modified-time', value: 'mod-time' },
        operators: SearchUtil.numberOperators,
        valueType: ValueType.inputField
      }
    ];
  }

  protected populate(inputConfig: SearchSpec) {
    const inValues = inputConfig.in;
    const isValues = inputConfig.is;
    const hasValues = this.parseExpressionList(inputConfig.has, true);
    const tagValues = this.parseExpressionList(inputConfig.tag, false);

    if (inValues) {
      this.selectedCategories = inValues.split(',');
    }
    if (isValues) {
      this.selectedKinds = isValues.split(',');
    }

    if (hasValues.length > 0) {
      const list = this.buildFieldFormControlList(hasValues);
      this.fieldFormArray = new FormArray(list);
      this.isToSelectField = true;
    }

    if (tagValues.length > 0) {
      const list = this.buildLabelFormControlList(tagValues);
      this.labelFormArray = new FormArray(list);
      this.isToSelectLabel = true;
    }

  }

  protected buildFieldFormControlList(searchExpressons: SearchExpression[]): FormControl[] {
    const list = [];
    searchExpressons.filter((item) => {
      const formControl = new FormControl({ keyFormControl: item.key, operatorFormControl: item.operator, valueFormControl: item.values.join(',') });
      list.push(formControl);
    });
    return list;
  }

  protected buildLabelFormControlList(searchExpressons: SearchExpression[]): FormControl[] {
    const list = [];
    searchExpressons.filter((item) => {
      const op = item.operator;
      const formControl = new FormControl({ keyFormControl: 'text', operatorFormControl: op, valueFormControl: item.values.join(','), keytextFormName: item.key });
      list.push(formControl);
    });
    return list;
  }

  protected parseExpressionList(inputSearchString: any, isField: boolean): SearchExpression[] {
    const list = [];
    if (!inputSearchString) {
      return [];
    }
    const values = inputSearchString.split(',');
    for (let i = 0; i < values.length; i++) {
      const exprStr = values[i];
      const expr = SearchUtil.parseToExpression(exprStr, isField);
      if (expr) {
        list.push(expr);
      }
    }
    return list;
  }

  // categories
  filterCategorySuggestions(event) {
    const query = event.query;
    const cats = SearchUtil.getCategories();
    const suggestedCats = [];
    cats.filter((item) => {
      if (item.toLowerCase().indexOf(query.toLowerCase()) >= 0) {
        if (this.selectedCategories.indexOf(item) < 0) {
          suggestedCats.push(item);
        }
      }
    });
    if (suggestedCats.length > 0) {
      this.categorySuggestions.length = 0;
      this.categorySuggestions = suggestedCats;
    }
  }

  getAvailableCategories(): any[] {
    const cats = SearchUtil.getCategories();
    const availableCategories = [];
    cats.filter((item) => {
      if (this.selectedCategories.indexOf(item) < 0) {
        availableCategories.push(item);
      }
    });
    return availableCategories;
  }

  // kinds

  filterkindSuggestions(event) {
    const query = event.query;
    const kinds = SearchUtil.getKinds();
    const suggestedKinds = [];
    kinds.filter((item) => {
      if (item.toLowerCase().indexOf(query.toLowerCase()) >= 0) {
        if (this.selectedKinds.indexOf(item) < 0) {
          suggestedKinds.push(item);
        }
      }
    });
    if (suggestedKinds.length > 0) {
      this.kindSuggestions.length = 0;
      this.kindSuggestions = suggestedKinds;
    }
  }

  getAvailableKinds(): any[] {
    let newKindList = [];
    if (this.selectedCategories.length > 0) {
      for (let i = 0; i < this.selectedCategories.length; i++) {
        const cat = this.selectedCategories[i];
        const kindByCat = SearchUtil.getKindsByCategory(cat);
        newKindList = newKindList.concat(kindByCat);
      }
      if (newKindList.length > 0) {
        newKindList = newKindList.sort();
      }
    } else {
      newKindList = SearchUtil.getKinds();
    }
    const availableKinds = [];
    newKindList.filter((item) => {
      if (this.selectedKinds.indexOf(item) < 0) {
        availableKinds.push(item);
      }
    });
    return availableKinds;
  }

  /**
   * This API serves html template
   */
  handleFieldRepeaterData(values: any) {
    this.fieldOutput = values;
  }

  /**
   * This API serves html template
   */
  handleLabelRepeaterData(values: any) {
    this.labelOutput = values;
  }

  /**
   * This API serves html template
   */
  onCancelGuidedSearchClick($event) {
    this.selectedCategories.length = 0;
    this.selectedKinds.length = 0;
    if (this.fieldOutput) {
      this.fieldOutput.length = 0;
    }
    if (this.labelOutput) {
      this.labelOutput.length = 0;
    }
  }

  /**
   * This API serves html template
   * Make up a search-spec object and emit an event.  Let upper class to handle the search.
   */
  onProceedGuidedSearchClick($event) {
    const obj: GuidedSearchCriteria = {
      in: this.selectedCategories,
      is: this.selectedKinds,
      has: (this.fieldRepeater) ? this.fieldRepeater.getValues() : [],
      tag: (this.labelRepeater) ? this.labelRepeater.getValues() : []
    };
    this.proceedGuidedSearch.emit(obj);
  }

  /**
   * This API serves html template
   */
  setShowSelectField(event) {
    this.isToSelectField = !this.isToSelectField;
  }

  /**
   * This API serves html template
   */
  setShowSelectLabel(event: any) {
    this.isToSelectLabel = !this.isToSelectLabel;
  }


  onKindSelectionCheck($event, option, selected) {
    if (this.isToSelectField) {
      return;
    }
    const kind = (this.selectedKinds && this.selectedKinds[0]) ? this.selectedKinds[0] : null;
    if (!kind) {
      return;
    }
  }

   /**
    * The fieldData looks like below
     {
      key: { label: 'name', value: 'name' },
      operators: [
        { label: 'equal', value: 'equal' },
        { label: 'not equal', value: 'not equal' },
        { label: 'in', value: 'in' },
        { label: 'not in', value: 'not in' }
      ],
      valueType: ValueType.inputField
    }
     */
  getFieldData(): any {
    const kind = (this.selectedKinds && this.selectedKinds[0]) ? this.selectedKinds[0] : null;
    if (!kind) {
      return this.fieldData;
    }
    const modelData: SearchModelField = SearchUtil.getModelInfoByKind(kind);
    let fieldData = [];
    const keys = Object.keys(modelData);
    keys.filter((key) => {
        const keyData = this.getFieldDataHelper(kind, modelData, key);
        fieldData = fieldData.concat(keyData);
    });


    return fieldData;
  }

  getFieldDataHelper(kind: string, modelData: SearchModelField, key: string): any[] {
    const fieldData = [];
    modelData[key].filter((item) => {
      if (this.acceptField(key, item)) {
        const config = {
          key: {
            label: key + '.' + item,
            value:  key + '.' + item,
          },
          operators: this.getFieldOperators(kind, key, item),
          valueType: ValueType.inputField
        };
        fieldData.push(config);
      }
    });
    return fieldData;
  }

  acceptField(key, item): boolean {
    if (key === SearchUtil.SEARCHFIELD_META) {
      if (item === 'labels' || item === 'self-link') {
        return false;
      }
    }
    return true;
  }

  getFieldOperators(kind: string, key: string, subKey: string): any[] {
   return SearchUtil.getOperators(kind, key, subKey);
  }

  getMetaOperators(item: any): any[] {
    return Utility.getLodash().union(SearchUtil.stringOperators, SearchUtil.numberOperators);
  }

  getSpecOperators(item: any): any[] {
    return this.getMetaOperators(item);
  }

  getStatusOperators(item: any): any[] {
    return this.getMetaOperators(item);
  }

  /**
   * Buidl place-holder text for repeater-item
   *
   * @param repeater
   * @param keyFormName
   */
  buildFieldValuePlaceholder(repeater: RepeaterItem, keyFormName: string) {
    // TODO: may change this once we have enhanced category-mapping.ts
    const key = repeater.formGroup.value[keyFormName];
    if (key.startsWith(SearchUtil.SEARCHFIELD_META)) {
      if (key.indexOf('time') > -1) {
        return 'YYYY-MM-DDTHH:mm:ss.sssZ';
      }
    }
    if (key.startsWith(SearchUtil.SEARCHFIELD_SPEC)) {
      if (key.indexOf('-ip') >  -1) {
        return 'xxx.xxx.xxx.xxx';
      }
    }
    if (key.startsWith(SearchUtil.SEARCHFIELD_STATUS)) {
      if (key.indexOf('time') > -1 ) {
        return 'YYYY-MM-DDTHH:mm:ss.sssZ';
      }
      if (key.indexOf('date') > -1 ) {
        return 'YYYY-MM-DD';
      }
    }
    return key;
  }

}
