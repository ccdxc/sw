import { Component, EventEmitter, Input, Output, OnInit, ViewEncapsulation, ViewChild, OnChanges, AfterViewInit, AfterViewChecked, DoCheck } from '@angular/core';
import { SearchUtil } from '@components/search/SearchUtil';

/**
 * This component displays suggestions for global search widget. search.component.html includes this component.
 */
@Component({
  selector: 'app-searchsuggestions',
  templateUrl: './searchsuggestions.component.html',
  styleUrls: ['./searchsuggestions.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SearchsuggestionsComponent implements OnInit, OnChanges, AfterViewChecked {

  @Input() suggestions: any[];
  @Output() suggestionItemClick: EventEmitter<any> = new EventEmitter();

  highlightOption: any;
  highlightOptionChanged = false;

  constructor() { }

  ngOnInit() { }

  ngOnChanges() {  }

  private hightlightListOption(index: number) {
    const list = document.getElementsByClassName('searchsuggestion-option-div');
    if (list) {
      const element = <HTMLElement>list[0];
      if (element) {
        element.focus();
      }
      this.highlightOption = this.suggestions[index];
    }
  }

  ngAfterViewChecked() {
    if (!this.highlightOption) {
      setTimeout(() => {
        this.hightlightListOption(0);
      }, 1);
    }
  }

  findOptionIndex(option): number {
    let index = -1;
    if (this.suggestions) {
      for (let i = 0; i < this.suggestions.length; i++) {
        if ((option === this.suggestions[i])) {
          index = i;
          break;
        }
      }
    }
    return index;
  }

  onKeydown(event) {
    this.keydownHandler(event);
  }

  /**
   * Search.component will pass keydown handling logic to this API
   * @param event
   *
   */
  keydownHandler(event) {
    const highlightItemIndex = this.findOptionIndex(this.highlightOption);

    switch (event.which) {
      // down
      case SearchUtil.EVENT_KEY_DOWN:
        if (highlightItemIndex !== -1) {
          const nextItemIndex = highlightItemIndex + 1;
          if (nextItemIndex !== (this.suggestions.length)) {
            this.highlightOptionChanged = true;
            this.hightlightListOption(nextItemIndex);
            this.highlightOption = this.suggestions[nextItemIndex];
          }
        } else {
          this.hightlightListOption(0);
          this.highlightOption = this.suggestions[0];
        }

        event.preventDefault();
        break;

      // up
      case SearchUtil.EVENT_KEY_UP:
        if (highlightItemIndex > 0) {
          const prevItemIndex = highlightItemIndex - 1;
          this.highlightOptionChanged = true;
          this.hightlightListOption(prevItemIndex);
          this.highlightOption = this.suggestions[prevItemIndex];
        }
        event.preventDefault();
        break;

      // enter
      case SearchUtil.EVENT_KEY_ENTER:
      // right
      case SearchUtil.EVENT_KEY_RIGHT:
        if (this.highlightOption) {
          this.selectItem(event, this.highlightOption);
        }
        event.preventDefault();
        break;
    }
  }

  getClassName(): string {
    return this.constructor.name;
  }

  displayItem(item): string {
    return JSON.stringify(item, null, 2);
  }

  onOptionClick($event, option) {
    if (option) {
      this.selectItem($event, option);
      this.highlightOption = option;
      this.highlightOptionChanged = true;
    }
  }

  selectItem($event, option) {
    this.suggestionItemClick.emit({
      event: $event,
      option: option
    }
    );
  }

}
