/******************************************************************                                                                       
     	Search packages:                                                 
                                                                       
                                                                       
	Sourcecode and documentation for ebook-tools version 0.1.1-3         
	show bar | Show file versions 	                                     
                                                                       
    * Main Page                                                        
    * Classes                                                          
    * Files                                                            
    * Directories                                                      
                                                                       
ebook-tools-0.1.1 ? src ? libepub                                      
epub_shared.h                                                          
*******************************************************************/                                                                       
#ifndef EPUB_SHARED_H                                            
#define EPUB_SHARED_H 1                                          
                                                                 
/**                                                              
   Metadata types                                                
 */                                                              
enum epub_metadata {                                             
  EPUB_ID, /**< ebook id*/                                       
  EPUB_TITLE, /**< ebook title*/                                 
  EPUB_CREATOR, /**< ebook creator*/                             
  EPUB_CONTRIB, /**< ebook contributor*/                         
  EPUB_SUBJECT, /**< ebook subject*/                             
  EPUB_PUBLISHER, /**< ebook publisher*/                         
  EPUB_DESCRIPTION, /**< ebook description*/                     
  EPUB_DATE, /**< ebook data */                                  
  EPUB_TYPE, /**< ebook type */                                  
  EPUB_FORMAT, /**< ebook format */                              
  EPUB_SOURCE, /**< ebook source */                              
  EPUB_LANG, /**<  ebook language */                             
  EPUB_RELATION, /**< ebook relation*/                           
  EPUB_COVERAGE, /**< ebook coverage*/                           
  EPUB_RIGHTS,/**< ebook rights */                               
  EPUB_META, /**< ebook extra metadata*/                         
};                                                               
                                                                 
/**                                                              
   Ebook Iterator types                                          
*/                                                               
enum eiterator_type {                                            
  EITERATOR_SPINE, /**< all the spine */                         
  EITERATOR_LINEAR, /**< the linear reading spine parts */       
  EITERATOR_NONLINEAR, /**< the non linear reading spine parts */
  //  EITERATOR_TOUR,                                            
};                                                               
                                                                 
/**                                                              
   Ebook Table Of Content Iterator types                         
*/                                                               
enum titerator_type {                                            
  TITERATOR_NAVMAP, /**< Navigation map  */                      
  TITERATOR_GUIDE, /**< Guide to the ebook parts */              
  TITERATOR_PAGES, /**< The pages of the ebook */                
};                                                               
                                                                 
#endif
