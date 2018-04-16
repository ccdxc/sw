import { Component, EventEmitter, Input, Output, OnInit, ViewEncapsulation, ViewChild, OnChanges, AfterViewInit, AfterViewChecked, DoCheck } from '@angular/core';
import { ElementRef, Renderer2, IterableDiffers, ChangeDetectorRef, NgZone, forwardRef, ContentChildren, QueryList, TemplateRef } from '@angular/core';
import { NG_VALUE_ACCESSOR, ControlValueAccessor } from '@angular/forms';

import { DomHandler, AutoCompleteModule } from 'primeng/primeng';
import { AutoComplete } from 'primeng/autocomplete';
import { SharedModule, PrimeTemplate } from 'primeng/components/common/shared';

import { ObjectUtils } from 'primeng/components/utils/objectutils';

import { ModuleUtility } from '../ModuleUtility';
/**
 * pw-searchbox extends from p-autocomplete of primeNG
 * We need the template capability of p-autocomplete
 * 
 * The template includes <span class="pw-searchbox"...
 * /Users/jeffhu/src/github.com/pensando/sw/venice/ui/web-app-framework/ng-package.json has special settings for packaging widgetModule
 *  
 * app.component.ts has import { PrimengModule } from './primeng.module';
 */
@Component({
  selector: 'pw-searchbox',
  templateUrl: './searchbox.component.html',
  styleUrls: ['./searchbox.component.css'],
  host: {
    '[class.ui-inputwrapper-filled]': 'filled',
    '[class.ui-inputwrapper-focus]': 'focus'
  },
  providers: [{
    provide: NG_VALUE_ACCESSOR,
    useExisting: forwardRef(() => SearchboxComponent),
    multi: true
  }, DomHandler, ObjectUtils]
})
export class SearchboxComponent extends AutoComplete implements OnInit, OnChanges {

  protected id_prefix = 'pw-searchbox_';
  @Input() id: string = null;
  @Input() templateSwitchIndicator: string = 'pensando';

  @Output() invokeSearch: EventEmitter<any> = new EventEmitter();

  constructor(public el: ElementRef, public domHandler: DomHandler, public differs: IterableDiffers,
    public renderer: Renderer2, public objectUtils: ObjectUtils, public changeDetector: ChangeDetectorRef,
  ) {

    super(el, domHandler, renderer, objectUtils, changeDetector, differs);
  }

  ngOnInit() {

  }

  ngOnChanges() {
    if (!this.id) {
      this.id = this.id_prefix + ModuleUtility.s4();
    }
  }

  /**
   * Override super.api
   * Say value = [1, 2, 4, 5, 7], I am removing 4
   * It will return [1, 2]
   * 
   * @param item 
   */
  removeItem(item: any) {
    let itemIndex = this.domHandler.index(item);
    let removedValue = this.value[itemIndex];
    this.value = this.value.filter((val, i) => i < itemIndex);
    this.onModelChange(this.value);
    this.onUnselect.emit(removedValue);
  }
  removeAllItem($event) {
    this.value.length = 0;
    this.onModelChange(this.value);
  }

  onInvokeSearch($event) {
    console.log("SearchboxComponent.invokeSearch()", this.value);
    this.invokeSearch.emit(this.value);
  }
  /**
   *  Override super's API
  
  hide() {
    this.panelVisible = !this._shouldCloseSuggestionPanel();
    this.unbindDocumentClickListener();
  }
  // */

  _shouldCloseSuggestionPanel(): boolean {
    return false;
  }

  /**
   * Override Super.API
   *
   */

  selectItem(option: any, focus: boolean = true) {
    if (this.multiple && !option.selectedOption) {
      return;
    }
    if (this.multiple) {

      this.multiInputEL.nativeElement.value = '';
      this.value = this.value || [];
      if (!this.isSelected(option)) {
        this.value = [...this.value, option];
        this.onModelChange(this.value);
      }
    }
    else {
      this.inputEL.nativeElement.value = this.field ? this.objectUtils.resolveFieldData(option, this.field) || '' : option;
      this.value = option;
      this.onModelChange(this.value);
    }

    this.onSelect.emit(option);
    this.updateFilledState();
    this._suggestions = null;

    if (focus) {
      this.focusInput();
    }
  }

  /**
   * Override Super.API
   * 
   */
  search(event: any, query: string) {
    //allow empty string but not undefined or null
    if (query === undefined || query === null) {
      return;
    }

    this.loading = true;

    this.completeMethod.emit({
      originalEvent: event,
      query: query,
      widget: this
    });
  }


}
