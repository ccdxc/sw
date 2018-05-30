import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AlertlistitemComponent } from './alertlistitem.component';

describe('AlertlistitemComponent', () => {
  let component: AlertlistitemComponent;
  let fixture: ComponentFixture<AlertlistitemComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [AlertlistitemComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertlistitemComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
