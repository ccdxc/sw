import { Component, EventEmitter, Input, Output, OnInit, ViewEncapsulation, OnChanges, ViewChild } from '@angular/core';
import { ElementRef, Renderer2, IterableDiffers, ChangeDetectorRef, forwardRef } from '@angular/core';
import { NG_VALUE_ACCESSOR } from '@angular/forms';

import { DomHandler } from 'primeng/primeng';
import { AutoComplete } from 'primeng/autocomplete';
import { ObjectUtils } from 'primeng/components/utils/objectutils';

import { Utility } from '@app/common/Utility';

import { SearchsuggestionsComponent } from './searchsuggestions.component';
import { SearchUtil } from '@app/common/SearchUtil';

/**
 * SearchComponent extends PrimeNG AutoComplete component
 * This widget is hosted inside searchbox.component.ts.  It also includes a searchsuggestions.component.
 *
 * note: component.host: registers document-click. It works with onSearchWidgetFocusOut($event) to handle widget focus out event.
 * If user clicks outside of the widget, it will hide the suggestion-panel
 *
 * Note:
 * To support keyboard up/down/enter key naviation, when there is a keydown event, we delegate the keyDown handling to the underneath searchsuggestions.component
 */

@Component({
  selector: 'app-search',
  templateUrl: './search.component.html',
  styleUrls: ['./search.component.scss'],
  encapsulation: ViewEncapsulation.None,
  host: {
    '(document:click)': 'onDocumentClick($event)',  // listens to document click
    '[class.ui-inputwrapper-filled]': 'filled',
    '[class.ui-inputwrapper-focus]': 'focus'
  },
  providers: [{
    provide: NG_VALUE_ACCESSOR,
    useExisting: forwardRef(() => SearchComponent),
    multi: true
  }, DomHandler, ObjectUtils]
})
export class SearchComponent extends AutoComplete implements OnInit, OnChanges {
  @ViewChild('suggestionWidget') _suggestionWidget: SearchsuggestionsComponent;

  protected id_prefix = 'app-searchbox_';
  @Input() id: string = null;
  @Input() templateSwitchIndicator = 'pensando';
  @Input() searchsuggestions: any[];

  @Output() invokeSearch: EventEmitter<any> = new EventEmitter();
  @Output() invokeSuggestionOnSearchClick: EventEmitter<any> = new EventEmitter();

  _lastWindowClickEvent;

  constructor(public el: ElementRef, public domHandler: DomHandler, public differs: IterableDiffers,
    public renderer: Renderer2, public objectUtils: ObjectUtils, public changeDetector: ChangeDetectorRef,
  ) {
    super(el, domHandler, renderer, objectUtils, changeDetector, differs);
  }

  ngOnInit() {

  }

  ngOnChanges() {
    if (!this.id) {
      this.id = this.id_prefix + Utility.s4();
    }
  }

  getClassName(): string {
    return this.constructor.name;
  }

  /**
   * Whenever user clicks on documment, we remember the click event.  It is needed for widget.focusout handling.
   */
  onDocumentClick($event) {
    this._lastWindowClickEvent = $event;
    if (this.panelVisible) {
      if (!this.isFocusWithinWidget(this.el.nativeElement)) {
        this.panelVisible = false;
        this.unbindDocumentClickListener();
      }
    }
  }

  /**
   * This API controls whether to hide the suggestion panel.
   * If user's last click is within search-widget, we will keep the suggestion panel open.
   *
   *
   */
  onSearchWidgetFocusOut($event) {
    if (!this.isFocusWithinWidget($event.currentTarget)) {
      this.panelVisible = false;
      this.unbindDocumentClickListener();
    }
  }

  /**
   * Traversing up the DOM to see if user's last click event.target is under input target
   * @param target
   */
  private isFocusWithinWidget(target): boolean {
    if (!this._lastWindowClickEvent) {
      return false;
    }
    let parentElement = this._lastWindowClickEvent.target;
    while (parentElement) {
      if (parentElement === target) {
        return true;
      }
      const classlist = parentElement.classList;
      if (classlist && classlist[0] && classlist[0] === 'searchbox-panel-div') {
        return true;
      }
      parentElement = parentElement.parentNode;
    }
    return false;
  }

  /**
   * Invoke search
   * If input parameter isSuggestionMode === true.  UI will arrange search-response in suggestion panel. Otherwise, UI will display Search-Result UI.
   * @param $event
   * @param isSuggestionMode
   */
  onInvokeSearch($event, isSuggestionMode: boolean = true) {
    console.log('SearchboxComponent.invokeSearch()', this.value);
    this.invokeSearch.emit({
      text: this.getInputText(),
      mode: isSuggestionMode
    }
    );
  }

  /**
   * Override Super.API
   *
   */
  selectItem(event: any, option: any, focus: boolean = true) {
    if (this.multiple) {
      this.value = this.value || [];
      if (!this.isSelected(option)) {
        this.value = [...this.value, option];
        this.onModelChange(this.value);
      }
    } else {
      this.inputEL.nativeElement.value = this.field ? this.objectUtils.resolveFieldData(option, this.field) || '' : option;
      this.value = option;
      this.onModelChange(this.value);
    }

    this.onSelect.emit(option);
    this.updateFilledState();
    this._suggestions = null;
    // when select an option, when don't want to hide suggestion-panel yet. // this.hide();
    if (focus) {
      this.focusInput();
    }
  }

  /**
   *
   * @param $event
   * @param option
   */
  onSearchSuggestionItemSelect($event) {
    this.selectItem($event.event, $event.option);
  }

  /**
   * Override Super.API
   *
   */
  search(event: any, query: string) {
    // allow empty string but not undefined or null
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

  /**
   * Override super.api
   * @param event
   *
   * We want to invoke seach only when enter-key is pressed.
   */
  onKeydown(event) {
    switch (event.which) {
      case SearchUtil.EVENT_KEY_ENETER:
        const query = this.getInputText();
        this.onInvokeSearch(event, true);  // invoke search to get get search suggestions. (2nd parameter is true)
      break;
      case  SearchUtil.EVENT_KEY_LEFT:
      case  SearchUtil.EVENT_KEY_UP:
      case  SearchUtil.EVENT_KEY_RIGHT:
      case  SearchUtil.EVENT_KEY_DOWN:
        this._suggestionWidget.onKeydown(event);
        break;
      case  SearchUtil.EVENT_KEY_TAB:
         this.onInvokeSearch(event, true);
         break;
      default:
        super.onKeydown(event);
        break;

    }
  }

  /**
  * Override Super
  * @param event
  */
  onInputClick(event: MouseEvent) {
    super.onInputClick(event);
    if (!this.suggestions || this.suggestions.length === 0) {
      this.invokeSuggestionOnSearchClick.emit({
        originalEvent: event
      });
    } else {
      if (!this.panelVisible) {
        this.panelVisible = true;
      }
    }
  }

  /**
   * Override super's
   * for debugging. don't hide suggestion-panel
   */
  hide() {
    this.panelVisible = false;
    this.unbindDocumentClickListener();
  }

  /**
   * Override super's
   * @param event
   */
  onInputBlur(event) {
    if (!this.focus) {
      this.panelVisible = false;
    }
    this.focus = false;
    this.onModelTouched();
    this.onBlur.emit(event);
  }


  /**
   * This API serves html template
   */
  getKeywords() {
    const value = this.getInputText();
    if (value.length > 50) {
      return value.substring(0, 49) + ' ...';
    }
    return this.getInputText();
  }

  getKeywordsTooltip(): string {
    return this.getInputText();
  }

  /**
   * This API serves html template
   */
  showKeywordDiv(): boolean {
    return (this.getInputText().length > 0);
  }

  public setInputText(text: string) {
    if (this.multiInputEL) {
      this.multiInputEL.nativeElement.value = text;
    }
  }

  public getInputText(): string {
    return (this.multiInputEL) ? this.multiInputEL.nativeElement.value : '';
  }

  /**
   * This API serves html template
   */
  displayItem(item): string {
    return JSON.stringify(item, null, 2);
  }

  /**
   * This API serves html template
   */
  showSearchIcons(): boolean {
    const value = this.getInputText();
    return (!this.disabled && this.multiple && value && value.length > 0);
  }

  onSearchButtonKeyDown(event) {
    if (event && event.which === 13) {
      this.onInvokeSearch(event, false);
    }
  }

  /**
   * remove all selections
   */
  clearSearchInputText($event) {
    this.setInputText('');
    if (this.value) {
      this.value.length = 0;
    }
    if (this.suggestions) {
      this.suggestions.length = 0;
    }
    this.onModelChange(this.value);
  }
}

