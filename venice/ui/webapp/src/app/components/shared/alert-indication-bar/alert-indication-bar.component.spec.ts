import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AlertIndicationBarComponent } from './alert-indication-bar.component';

describe('AlertIndicationBarComponent', () => {
  let component: AlertIndicationBarComponent;
  let fixture: ComponentFixture<AlertIndicationBarComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      imports: [],
      declarations: [AlertIndicationBarComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertIndicationBarComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create AlertIndicationBarComponent', () => {
    expect(component).toBeTruthy();
  });

});
