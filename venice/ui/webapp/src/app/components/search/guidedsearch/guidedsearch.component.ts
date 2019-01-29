import { Component, EventEmitter, Input, OnChanges, OnInit, Output, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormArray, FormControl } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { GuidedSearchCriteria, SearchExpression, SearchSpec, SearchModelField } from '@app/components/search';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { RepeaterComponent, RepeaterData, ValueType } from 'web-app-framework';

@Component({
  selector: 'app-guidedsearch',
  templateUrl: './guidedsearch.component.html',
  styleUrls: ['./guidedsearch.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class GuidesearchComponent implements OnInit, OnChanges {

  @ViewChild('fieldRepeater') fieldRepeater: FieldselectorComponent;
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
    this.categorySuggestions = Utility.getCategories();
    this.kindSuggestions = Utility.getKinds();
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

  populateHelper_Kind(isValues: string): any {
    const list = isValues.split(',');
    const output = [];
    list.filter((kind) => {
      if (SearchUtil.isValidKind(kind)) {
        output.push(kind);
      }
    });
    return output;
  }

  populateHelper_Category(inValues: string): any {
    const list = inValues.split(',');
    const output = [];
    list.filter((kind) => {
      if (SearchUtil.isValidCategory(kind)) {
        output.push(kind);
      }
    });
    return output;
  }



  /**
   * Populate guided-search panel
   */
  protected populate(inputConfig: SearchSpec) {
    const inValues = inputConfig.in;
    const isValues = inputConfig.is;

    if (inValues) {
      this.selectedCategories = this.populateHelper_Category(inValues);
    }
    if (isValues) {
      this.selectedKinds = this.populateHelper_Kind(isValues);
    }

    const isEvent = this.selectedKinds.includes('Event');
    const hasValues = this.parseExpressionList(inputConfig.has, true, isEvent);
    const tagValues = this.parseExpressionList(inputConfig.tag, false);

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
    searchExpressons.forEach((item) => {
      // We want to support multi-select as well as delimited string (xxx,yyy). So we use item.values instead of item.values.join(',').
      const formControl = new FormControl({ keyFormControl: item.key, operatorFormControl: item.operator, valueFormControl: (item.values) ? item.values : [] });
      list.push(formControl);
    });
    return list;
  }

  protected buildLabelFormControlList(searchExpressons: SearchExpression[]): FormControl[] {
    const list = [];
    searchExpressons.forEach((item) => {
      const op = item.operator;
      const formControl = new FormControl({ keyFormControl: 'text', operatorFormControl: op, valueFormControl: (item.values) ? item.values : [], keytextFormName: item.key });
      list.push(formControl);
    });
    return list;
  }

  protected parseExpressionList(inputSearchString: any, isField: boolean, isEvent = false): SearchExpression[] {
    const list = [];
    if (!inputSearchString) {
      return [];
    }
    const values = inputSearchString.split(',');
    let prevExp: SearchExpression = null;
    // suppor case like "has:name=~Liz,test,tenant=default"
    for (let i = 0; i < values.length; i++) {
      const exprStr = values[i];
      const expr: SearchExpression = SearchUtil.parseToExpression(exprStr, isField, isEvent);
      if (expr) {
        list.push(expr);
        prevExp = expr;
      } else {
        if (prevExp) {
          prevExp.values.push(exprStr);
        }
      }
    }
    return list;
  }

  // categories
  filterCategorySuggestions(event) {
    const query = event.query;
    const cats = Utility.getCategories();
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
    const cats = Utility.getCategories();
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
    const kinds = Utility.getKinds();
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
        const kindByCat = Utility.getKindsByCategory(cat);
        newKindList = newKindList.concat(kindByCat);
      }
      if (newKindList.length > 0) {
        newKindList = newKindList.sort();
      }
    } else {
      newKindList = Utility.getKinds();
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
    this.isAllInputsValidated();
  }

  /**
   * This API serves html template
   * Make up a search-spec object and emit an event.  Let upper class to handle the search.
   */
  onProceedGuidedSearchClick($event) {
    const obj: GuidedSearchCriteria = this.genGuidedSearchCriteria();
    const isEmpty = SearchUtil.isGuidedSearchCriteriaEmpty(obj);
    if (!isEmpty) {
         this.proceedGuidedSearch.emit(obj);
    }
  }

  private genGuidedSearchCriteria(): GuidedSearchCriteria {
    return {
      in: this.selectedCategories,
      is: this.selectedKinds,
      has: (this.fieldRepeater) ? this.fieldRepeater.getValues() : [],
      tag: (this.labelRepeater) ? this.labelRepeater.getValues() : []
    };
  }

  isAllInputsValidated(): boolean {
    const obj: GuidedSearchCriteria = this.genGuidedSearchCriteria();
    const isEmpty = SearchUtil.isGuidedSearchCriteriaEmpty(obj);
    return !isEmpty;
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
    keys.forEach((key) => {
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
            value: key + '.' + item,
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
    return SearchUtil.getOperators(kind, [key, subKey]);
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


}
