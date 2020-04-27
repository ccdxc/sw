import {
  Component,
  OnInit,
  ViewEncapsulation,
  Input,
  OnChanges,
  SimpleChanges,
  Output,
  EventEmitter,
} from '@angular/core';
import {LabelEditorMetadataModel, LabelEditorModel, VeniceObject} from './index';
import {Utility} from '@common/Utility';
import {FormArray, FormControl, FormGroup} from '@angular/forms';

// Label Editor Widget
// This widget will allow user to edit the key value pair in meta.labels for multiple objects at the same time.
// Example usage: webapp/src/app/components/cluster-group/naples.component.html

// Initially we iterate over the labels of all the selected objects and find the common labels.
// These common labels are displayed by the label editor in case the user wants to change or remove them.
// The user can also add new labels to all of these objects.
// If the user is trying to edit an existing key, we show a warning next to the editor and show more details about the conflict on hover.
// When the user saves the labels a list of the updated objects is sent back to the original component.

@Component({
  selector: 'app-labeleditor',
  templateUrl: './labeleditor.component.html',
  styleUrls: ['./labeleditor.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class LabeleditorComponent implements OnInit, OnChanges {
  models: LabelEditorModel[];
  usableArray = [];
  usableJson = {};
  labelWarnings = [null];
  commonLabels: LabelEditorModel[];
  updatedObjects: VeniceObject[] = [];

  form = new FormGroup({
    cities: new FormArray([
      new FormControl(''),
    ]),
  });

  @Input() dialogMode = true;

  @Input() objects: VeniceObject[] = [];
  // Instance propety used to get object "name"
  @Input() nameKey: string = 'meta.name';
  // the object user wants to edit. Must contains meta.labels
  @Input() metadata: LabelEditorMetadataModel;
  // In the future, some labels may be not editable and not deletable. Widget user can override this buildModel() API to customize it.
  @Input() myBuildModel: Function;
  // inLabelEditMode is contolling the dialog popup
  @Input() inLabelEditMode: boolean;


  // get edited meta.labels as an array format
  @Output() arrayValue = new EventEmitter();
  // get edited meta.labels as an obj format
  @Output() jsonValue = new EventEmitter();
  // save handler
  @Output() saveEmitter: EventEmitter<VeniceObject[]> = new EventEmitter<VeniceObject[]>();
  // cancel handler
  @Output() cancelEmitter: EventEmitter<object> = new EventEmitter<object>();

  @Input() saveLabelsOperationDone: boolean ;

  //  VS-1553, we want to disable save button when UI emits a call to update records. It works along with this.saveDSCProfileOperationDone
  disableSaveButton: boolean = false;

  formValidated: boolean = true;

  constructor() {
  }

  getClassName(): string {
    return this.constructor.name;
  }

  /**
   * Helper function to determine if obj exists and if the key is in the obj
   * @param obj
   * @param key
   */
  hasProp(obj, key): boolean {
    if (!obj) {
      return false;
    }
    return obj.hasOwnProperty(key);
  }

  onFormChange(prop, index) {
    this.onObjectEdit();
    this.updateWarning(prop, index);
  }

  /**
   * Called everytime when the row prop changed
   */
  onObjectEdit() {
    const array = [];
    const json = {};
    for (const prop of this.models) {
      prop.key = prop.key.trim();
      prop.value = prop.value.trim();
      const newProp = {key: prop.key, value: prop.value};
      if (prop.key === '' || prop.value === '') {
        continue;
      }
      array.push(newProp);
      json[prop.key] = prop.value;
    }

    this.arrayValue.emit(array);
    this.usableArray = array;
    this.jsonValue.emit(json);
    this.usableJson = json;
  }

  /**
   * Handle add row pop logic
   */
  addProperty() {
    this.models.push(this.buildModel('', ''));
    this.labelWarnings.push(null);
  }

  /**
   * Handle delete row prop logic
   * @param prop
   */
  delete(prop: LabelEditorModel) {
    const index: number = this.models.indexOf(prop);
    if (index === 0 && this.models.length === 1) {
      // Don't allow deletion of the last model, just clear its values out.
      this.models[0].key = '';
      this.models[0].value = '';
    } else if (index !== -1) {
      this.models.splice(index, 1);
    }
    this.onObjectEdit();
    this.labelWarnings.splice(index, 1);
  }

  /**
   * Helper function to get class for key input
   * @param prop
   */
  getKeyClass(prop: LabelEditorModel): string {
    if (!prop) {
      return '';
    }
    if (prop.keyClass) {
      return prop.keyClass;
    } else if (this.metadata.keysClasses) {
      return this.metadata.keysClasses;
    } else {
      return 'label-editor-input-key';
    }
  }

  getValueClass(prop: LabelEditorModel): string {
    if (!prop) {
      return '';
    }
    if (prop.valueClass) {
      return prop.valueClass;
    } else if (this.metadata.valuesClasses) {
      return this.metadata.valuesClasses;
    } else {
      return 'label-editor-input-value';
    }
  }

  /**
   * Helper function to get class for key input with error if the input is not valid
   * @param prop
   */
  getKeyClassWithValidator(prop: LabelEditorModel): string {
    return this.validate(prop.key) ? this.getKeyClass(prop) : `${this.getKeyClass(prop)} error`;
  }

  getValueClassWithValidator(prop: LabelEditorModel): string {
    return this.validateValue(prop.value) ? this.getValueClass(prop) : `${this.getValueClass(prop)} error`;
  }

  ngOnInit() {
    if (!this.models) {
      this.models = [];
    }
    this.init();
  }

  /**
   * Build LabelEditorModel models from meta.labels
   */
  buildModels(): LabelEditorModel[] {
    this.commonLabels = [];
    let labels = {};

    // Check if any objects are passed
    if (this.objects.length > 0 && !!this.objects[0]) {
      // Check if the first object has labels
      if (!!this.objects[0].meta.hasOwnProperty('labels') && !!this.objects[0].meta.labels) {
        labels = this.objects[0].meta.labels;
        // If multiple objects are passed, get all the common labels.
        for (const key of Object.keys(labels)) {
          if (this.isLabelCommon(key)) {
            this.commonLabels.push( this.buildModel(key, labels[key]) );
            this.removeCommonLabel(key);
          }
        }
      }
    }
    if (this.commonLabels.length === 0) {
      this.commonLabels.push(this.buildModel('', ''));
    }
    return Utility.getLodash().cloneDeep(this.commonLabels);
  }

  /**
   * Build LabelEditorModel from key and value
   * @param key
   * @param value
   */
  buildModel(key: string, value: string): LabelEditorModel {
    if (this.myBuildModel) {
      return this.myBuildModel(key, value);
    } else {
      return {
        key: key,
        keyEditable: true,
        value: value,
        valueEditable: true,
        element: 'input',
        inputType: 'text',
        deletable: true,
        keyPlaceholder: 'key',
        valuePlaceholder: 'value'
      } as LabelEditorModel;
    }
  }

  /**
   * Build default LabelEditorMetadataModel
   */
  buildDefaultMetadataModel(): LabelEditorMetadataModel {
    const myTitle = (this.objects.length === 1 && !!this.objects[0]) ? this.objects[0].meta.name + ' Labels' : 'Label Editor';
    const labelEditorMetadataModel = {
      title: myTitle,
      keysEditable: true,
      valuesEditable: true,
      propsDeletable: true,
      extendable: true,
      save: true,
      cancel: true,
    };
    return labelEditorMetadataModel;
  }

  ngOnChanges(changes: SimpleChanges) {

    if (this.saveLabelsOperationDone != null && this.saveLabelsOperationDone !== undefined  ) {
      this.disableSaveButton = false;
    }

    // the passed object(s) have changed. We need to process them again.
    if (this.objects.length === 0 || !this.objects[0]) {
      if (!this.models) {
        this.models = [
          this.buildModel('', '')
        ];
      }
    } else {
      this.init();
    }

    // When inLabelEditMode is false, editor is not actually destroyed just hidden
    // Previous warnings need to be reset when its hidden
    if (!!changes && changes.hasOwnProperty('inLabelEditMode') && !changes['inLabelEditMode'].currentValue) {
      for (let i = 0; i < this.labelWarnings.length; i++) {
        this.labelWarnings[i] = null;
      }
    }
  }

  /**
   * Data init
   */
  init() {
    if (!this.metadata) {
      this.metadata = this.buildDefaultMetadataModel();
    }
    this.updatedObjects = Utility.getLodash().cloneDeep(this.objects);
    this.usableJson = {};
    this.usableArray = [];
    this.models = this.buildModels();
    this.onObjectEdit();
  }

  isLabelCommon(key) {
    const valueSet = new Set<string>();
    for (const obj of this.objects) {
      if (!!obj.meta.labels && obj.meta.labels.hasOwnProperty(key)) {
        valueSet.add(obj.meta.labels[key]);
      } else {
        return false;
      }
      if (valueSet.size > 1) {
        return false;
      }
    }
    return true;
  }

  removeCommonLabel(key) {
    for (const obj of this.updatedObjects) {
      delete obj.meta.labels[key];
    }
  }

  // The new object retains all the untouched key-values from the old naples object.
  updateLabelsForObject(object, models) {
    if (!object.meta.hasOwnProperty('labels') || object.meta.labels === null ) {
      object.meta.labels = {};
    }
    for (const prop of models) {
      if (prop.key !== '' && prop.value !== '') {
        object.meta.labels[prop.key] = prop.value;
      }
    }
  }

  updateObjects() {
    for (const obj of this.updatedObjects) {
      this.updateLabelsForObject(obj, this.models);
    }
  }

  /**
   * Save button logic
   */
  save() {
    // save fields, the saveEmitter will handle the http call
    this.updateObjects();
    this.disableSaveButton = true; // disable [save] button to prevent user from clicking it multiple times
    this.saveEmitter.emit(this.updatedObjects);
  }

  /**
   * Cancel button logic
   */
  cancel() {
    // reset fields
    this.init();
    this.disableSaveButton = false;  // enable [save] button
    this.cancelEmitter.emit();
  }

  /**
   * Validate the input.
   * Currently support duplicate key detection.
   * @param key
   */
  validate(key: string): boolean {
    if (!!key || key === '') {
      return true;
    }
    let counter = 0;
    this.models.map(prop => {
      if (prop.key === key) { counter++; }
    });
    if (counter >= 2) {
      this.formValidated = false;
      return false;
    }
    this.formValidated = true;
    return true;
  }

  validateValue(value: string): boolean {
    if (value === '') {
      return true;
    }

    this.formValidated = !!value;
    return !!value;
  }

  /**
   * On add button keydown
   * @param $event
   */
  onAddKeydown($event) {
    if ($event && $event.which === 13) {
      this.addProperty();
    }
  }

  /**
   * On delete button keydown
   * @param $event
   * @param prop
   */
  onDeleteKeydown($event, prop: LabelEditorModel) {
    if ($event && $event.which === 13) {
      this.delete(prop);
    }
    if (this.models.length === 0) {
      this.models.push(this.buildModel('', ''));
    }
  }

  buildObjectLabelMap() {
    const objectLabelMap = new Map<string, Set<string>>();
    for (const object of this.objects) {
      if (object.meta.hasOwnProperty('labels') && !!object.meta.labels) {
        for (const key of Object.keys(object.meta.labels)) {
          if ( !(key in objectLabelMap)) {
            objectLabelMap[key] = new Set<string>();
          }
          objectLabelMap[key].add( Utility.getObjectValueByPropertyPath(object, this.nameKey) );
        }
      }
    }
    return objectLabelMap;
  }

  updateWarning(label, index) {
    // Currently we dont support empty keys or values
    if (label.key === '' || label.value === '') {
      this.labelWarnings[index] = 'Labels with empty key/value will be removed';
      return;
    }

    const objectLabelMap = this.buildObjectLabelMap();
    let tooltip = 'Key already present in ';
    if (label.key in objectLabelMap) {
      tooltip += Array.from(objectLabelMap[label.key]).join(', ');
    } else {
      tooltip = null;
    }
    this.labelWarnings[index] = tooltip;
  }
}
