import { DebugElement } from "@angular/core";
import { ComponentFixture } from "@angular/core/testing";

export class TestingUtility {
  fixture: ComponentFixture<any>;

  constructor(fixture: ComponentFixture<any>) {
    this.fixture = fixture;
  }

  setText(elem: DebugElement, text: string) {
    const elemNative = elem.nativeElement;
    elemNative.value = text;
    elemNative.dispatchEvent(new Event('input'));
    this.fixture.detectChanges();
    return this.fixture.whenStable();
  }

  sendEnterKeyup(elem: DebugElement) {
    elem.triggerEventHandler('keyup', { keyCode: 13 });
    this.fixture.detectChanges();
    return this.fixture.whenStable();
  }

  sendClick(elem: DebugElement) {
    const elemNative = elem.nativeElement;
    elemNative.click();
    elemNative.dispatchEvent(new Event('click'));
    this.fixture.detectChanges();
    return this.fixture.whenStable();
  }
}
